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

#ifndef ELEVATIONENGINE_H
#define ELEVATIONENGINE_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QObject>
#include <QHash>
#include <QVector>

// Local classes
class Settings;

// Qt classes
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class ElevationEngine : public QObject
{
    Q_OBJECT

public:
    enum Target {
        Image,
        Bookmark
    };

    explicit ElevationEngine(QObject *parent, Settings *settings);
    void request(Target target, const QVector<QString> &ids,
                 const QVector<KGeoTag::Coordinates> &coordinates);

signals:
    void lookupFailed(const QString &errorMessage) const;
    void notAllPresent(int locationsCount, int elevationsCount);
    void elevationProcessed(Target target, const QVector<QString> &ids,
                            const QVector<double> &elevations) const;

private slots:
    void processNextRequest();
    void cleanUpRequest(QNetworkReply *request);
    void processReply(QNetworkReply *reply);

private: // Functions
    void removeRequest(QNetworkReply *request);

private: // Variables
    struct RequestData
    {
        Target target;
        QVector<QString> ids;
    };

    Settings *m_settings;

    QNetworkAccessManager *m_manager;

    QTimer *m_requestTimer;
    QVector<Target> m_queuedTargets;
    QVector<QVector<QString>> m_queuedIds;
    QVector<QString> m_queuedLocations;

    QHash<QNetworkReply *, RequestData> m_requests;

};

#endif // ELEVATIONENGINE_H
