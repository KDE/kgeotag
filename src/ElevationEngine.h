// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#ifndef ELEVATIONENGINE_H
#define ELEVATIONENGINE_H

// Qt includes
#include <QObject>
#include <QHash>
#include <QVector>

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
    void request(Target target, const QVector<QString> &ids,
                 const QVector<Coordinates> &coordinates);

signals:
    void lookupFailed(const QString &errorMessage);
    void notAllPresent(int locationsCount, int elevationsCount);
    void elevationProcessed(Target target, const QVector<QString> &ids,
                            const QVector<double> &elevations);

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
