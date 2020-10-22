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

void ElevationEngine::request(const QString &path, const KGeoTag::Coordinates &coordinates)
{
    auto *request = m_manager->get(QNetworkRequest(QUrl(
        QStringLiteral("https://api.opentopodata.org/v1/aster30m?locations=%1,%2").arg(
                       QString::number(coordinates.lat), QString::number(coordinates.lon)))));

    m_requests.insert(request, path);
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
        emit lookupFailed();
        m_requests.remove(request);
    }
}

void ElevationEngine::processReply(QNetworkReply *request)
{
    if (! request->isOpen()) {
        // This happens if the request has been aborted by the cleanup timer
        return;
    }

    const auto id = m_requests.value(request);
    removeRequest(request);

    QJsonParseError error;
    const auto json = QJsonDocument::fromJson(request->readAll(), &error);
    if (error.error != QJsonParseError::NoError || ! json.isObject()) {
        emit lookupFailed();
        return;
    }

    const auto object = json.object();
    if (object.value(QStringLiteral("status")) != QStringLiteral("OK")) {
        emit lookupFailed();
        return;
    }

    const auto results = object.value(QStringLiteral("results")).toArray().at(0);
    if (results.isUndefined()) {
        emit lookupFailed();
        return;
    }

    const auto elevation = results.toObject().value(QStringLiteral("elevation"));
    if (elevation.isUndefined()) {
        emit lookupFailed();
        return;
    }

    emit elevationProcessed(id, elevation.toDouble());
}
