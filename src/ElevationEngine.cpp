/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "ElevationEngine.h"
#include "Settings.h"
#include "Coordinates.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

// C++ includes
#include <functional>

// opentopodata.org query restrictions
static constexpr int s_maximumLocations = 100;
static constexpr int s_msToNextRequest = 1000;

ElevationEngine::ElevationEngine(QObject *parent, Settings *settings)
    : QObject(parent),
      m_settings(settings)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &ElevationEngine::processReply);

    m_requestTimer = new QTimer(this);
    m_requestTimer->setSingleShot(true);
    m_requestTimer->setInterval(s_msToNextRequest);
    connect(m_requestTimer, &QTimer::timeout, this, &ElevationEngine::processNextRequest);
}

void ElevationEngine::request(ElevationEngine::Target target, const QVector<QString> &ids,
                              const QVector<Coordinates> &coordinates)
{
    // Check if we want to lookup different coordinates
    bool identicalCoordinates = true;
    if (coordinates.count() > 1) {
        const auto &firstCoordinates = coordinates.first();
        for (int i = 1; i < coordinates.count(); i++) {
            if (coordinates.at(i) != firstCoordinates) {
                identicalCoordinates = false;
                break;
            }
        }
    }

    if (identicalCoordinates) {
        // Add a request for one location
        m_queuedTargets.append(target);
        m_queuedIds.append(ids);
        const auto &firstCoordinates = coordinates.first();
        m_queuedLocations.append(QStringLiteral("%1,%2").arg(
                                                QString::number(firstCoordinates.lat()),
                                                QString::number(firstCoordinates.lon())));
    } else {
        // Create clusters of locations with at most s_maximumLocations locations per cluster

        QStringList locations;
        for (const auto &singleCoordinate : coordinates) {
            locations.append(QStringLiteral("%1,%2").arg(QString::number(singleCoordinate.lat()),
                                                         QString::number(singleCoordinate.lon())));
        }

        // Group all requested coordinates to groups with at most s_maximumLocations entries
        int start = 0;
        while (start < ids.count()) {
            m_queuedTargets.append(target);
            m_queuedIds.append(ids.mid(start, s_maximumLocations));
            m_queuedLocations.append(
                locations.mid(start, s_maximumLocations).join(QLatin1String("|")));
            start += s_maximumLocations;
        }
    }

    processNextRequest();
}

void ElevationEngine::processNextRequest()
{
    if (m_queuedTargets.isEmpty()) {
        // Nothing to do.
        // This happens because m_requestTimer always calls this after being finished.
        return;
    }

    if (m_requestTimer->isActive()) {
        // Pending request, we can't currently post another one.
        // m_requestTimer will call this again after having waited for s_msToNextRequest.
        return;
    }

    auto *reply = m_manager->get(QNetworkRequest(QUrl(
        QStringLiteral("https://api.opentopodata.org/v1/%1?locations=%2").arg(
                       m_settings->elevationDataset(), m_queuedLocations.takeFirst()))));
    m_requests.insert(reply, { m_queuedTargets.takeFirst(), m_queuedIds.takeFirst() });
    QTimer::singleShot(3000, this, std::bind(&ElevationEngine::cleanUpRequest, this, reply));

    // Block the next requst (checking)
    m_requestTimer->start();
}

void ElevationEngine::removeRequest(QNetworkReply *request)
{
    m_requests.remove(request);
    request->deleteLater();
}

void ElevationEngine::cleanUpRequest(QNetworkReply *request)
{
    if (m_requests.contains(request)) {
        request->abort();
        emit lookupFailed(i18n("The request timed out"));
        m_requests.remove(request);
    }
}

void ElevationEngine::processReply(QNetworkReply *request)
{
    if (! request->isOpen()) {
        // This happens if the request has been aborted by the cleanup timer
        return;
    }

    const auto [ target, ids ] = m_requests.value(request);
    removeRequest(request);

    const auto requestData = request->readAll();
    QJsonParseError error;
    const auto json = QJsonDocument::fromJson(requestData, &error);
    if (error.error != QJsonParseError::NoError || ! json.isObject()) {
        emit lookupFailed(i18n("Could not parse the server's response: Failed to create a JSON "
                               "document.</p>"
                               "<p>The error's description was: %1</p>"
                               "<p>The literal response was:</p>"
                               "<p><kbd>%2</kbd>", error.errorString(),
                               QString::fromLocal8Bit(requestData)));
        return;
    }

    const auto object = json.object();
    const auto statusValue = object.value(QStringLiteral("status"));
    if (statusValue.isUndefined()) {
        emit lookupFailed(i18n("Could not parse the server's response: Could not read the status "
                               "value"));
    }

    const auto statusString = statusValue.toString();
    if (statusString != QStringLiteral("OK")) {
        const auto errorValue = object.value(QStringLiteral("error"));
        const auto errorString = errorValue.isUndefined()
            ? i18n("Could not read error description") : errorValue.toString();
        emit lookupFailed(i18nc("A server error status followed by the error description",
                                "%1: %2", statusString, errorString));
        return;
    }

    const auto resultsValue = object.value(QStringLiteral("results"));
    if (! resultsValue.isArray()) {
        emit lookupFailed(i18n("Could not parse the server's response: Could not read the results "
                               "array"));
        return;
    }

    const auto resultsArray = resultsValue.toArray();
    bool allPresent = true;
    QVector<double> elevations;
    for (const auto result : resultsArray)  {
        const auto elevation = result.toObject().value(QStringLiteral("elevation"));
        if (elevation.isUndefined()) {
            emit lookupFailed(i18n("Could not parse the server's response: Could not read the "
                                   "elevation value"));
            return;
        } else if (elevation.isNull()) {
            allPresent = false;
        }
        elevations.append(elevation.toDouble());
    }

    const int originalElevationsCount = elevations.count();
    if (ids.count() > 1 && originalElevationsCount == 1) {
        // Same coordinates requested multiple times
        const auto coordinates = elevations.first();
        for (int i = 0; i < ids.count() - 1; i++) {
            elevations.append(coordinates);
        }
    }

    emit elevationProcessed(target, ids, elevations);

    if (! allPresent) {
        emit notAllPresent(ids.count(), originalElevationsCount);
    }
}
