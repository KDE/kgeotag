/* Copyright (C) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// Local includes
#include "ElevationEngine.h"

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

ElevationEngine::ElevationEngine(QObject *parent) : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &ElevationEngine::processReply);
}

void ElevationEngine::request(ElevationEngine::Target target, const QString &id,
                              const KGeoTag::Coordinates &coordinates)
{
    auto *request = m_manager->get(QNetworkRequest(QUrl(
        QStringLiteral("https://api.opentopodata.org/v1/aster30m?locations=%1,%2").arg(
                       QString::number(coordinates.lat), QString::number(coordinates.lon)))));

    m_requests.insert(request, { target, id });
    QTimer::singleShot(3000, this, std::bind(&ElevationEngine::cleanUpRequest, this, request));
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

    const auto [ target, id ] = m_requests.value(request);
    removeRequest(request);

    QJsonParseError error;
    const auto json = QJsonDocument::fromJson(request->readAll(), &error);
    if (error.error != QJsonParseError::NoError || ! json.isObject()) {
        emit lookupFailed(i18n("Could not parse the server's response: Failed to create a JSON "
                               "document"));
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

    const auto results = object.value(QStringLiteral("results")).toArray().at(0);
    if (results.isUndefined()) {
        emit lookupFailed(i18n("Could not parse the server's response: Could not read the results "
                               "array"));
        return;
    }

    const auto elevation = results.toObject().value(QStringLiteral("elevation"));
    if (elevation.isUndefined()) {
        emit lookupFailed(i18n("Could not parse the server's response: Could not read the "
                               "elevation value"));
        return;
    }

    emit elevationProcessed(target, id, elevation.toDouble());
}
