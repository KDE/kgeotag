// SPDX-FileCopyrightText: 2020-2023 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef GPXENGINE_H
#define GPXENGINE_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// Qt includes
#include <QObject>
#include <QVector>
#include <QHash>
#include <QDateTime>
#include <QImage>
#include <QJsonObject>

// Local classes
class GeoDataModel;

class GpxEngine : public QObject
{
    Q_OBJECT

public:
    enum LoadResult {
        OpenFailed,
        XmlError,
        NoGpxElement,
        NoGeoData,
        AlreadyLoaded,
        Okay
    };

    struct LoadInfo
    {
        LoadResult result;
        int tracks = 0;
        int segments = 0;
        int points = 0;
    };

    explicit GpxEngine(QObject *parent, GeoDataModel *geoDataModel);
    GpxEngine::LoadInfo load(const QString &path);
    Coordinates findExactCoordinates(const QDateTime &time, int deviation) const;
    Coordinates findInterpolatedCoordinates(const QDateTime &time, int deviation) const;
    QPair<Coordinates, QDateTime> findClosestTrackPoint(QDateTime time,
                                                        int cameraClockDeviation) const;
    void setMatchParameters(int exactMatchTolerance, int maximumInterpolationInterval,
                            int maximumInterpolationDistance);
    QByteArray lastDetectedTimeZoneId() const;
    bool timeZoneDataLoaded() const;

private: // Functions
    Coordinates findExactCoordinates(const QDateTime &time) const;
    Coordinates findInterpolatedCoordinates(const QDateTime &time) const;

private: // Variables
    GeoDataModel *m_geoDataModel;

    int m_exactMatchTolerance;
    int m_maximumInterpolationInterval;
    int m_maximumInterpolationDistance;

    QImage m_timezoneMap;
    double m_timezoneMapWidth = 0.0;
    double m_timezoneMapHeight = 0.0;
    QJsonObject m_timezoneMapping;
    QByteArray m_lastDetectedTimeZoneId;

};

#endif // GPXENGINE_H
