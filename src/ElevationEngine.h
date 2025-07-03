// SPDX-FileCopyrightText: 2020-2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef ELEVATIONENGINE_H
#define ELEVATIONENGINE_H

// Qt includes
#include <QObject>
#include <QHash>
#include <QList>

// Local classes
class Settings;
class Coordinates;

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
    void request(Target target, const QList<QString> &ids, const QList<Coordinates> &coordinates);

Q_SIGNALS:
    void lookupFailed(const QString &errorMessage);
    void notAllPresent(int locationsCount, int elevationsCount);
    void elevationProcessed(Target target, const QList<QString> &ids,
                            const QList<double> &elevations);

private Q_SLOTS:
    void processNextRequest();
    void cleanUpRequest(QNetworkReply *request);
    void processReply(QNetworkReply *reply);

private: // Functions
    void removeRequest(QNetworkReply *request);

private: // Variables
    struct RequestData
    {
        Target target;
        QList<QString> ids;
    };

    Settings *m_settings;

    QNetworkAccessManager *m_manager;

    QTimer *m_requestTimer;
    QList<Target> m_queuedTargets;
    QList<QList<QString>> m_queuedIds;
    QList<QString> m_queuedLocations;

    QHash<QNetworkReply *, RequestData> m_requests;

};

#endif // ELEVATIONENGINE_H
