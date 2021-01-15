/* SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

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
    void setMatchParameters(int exactMatchTolerance, int maximumInterpolationInterval,
                            int maximumInterpolationDistance);
    QByteArray lastDetectedTimeZoneId() const;

signals:
    void segmentLoaded(const QVector<Coordinates> &segment);

private: // Functions
    Coordinates findExactCoordinates(const QDateTime &time) const;
    Coordinates findInterpolatedCoordinates(const QDateTime &time) const;

private: // Variables
    GeoDataModel *m_geoDataModel;

    QVector<QDateTime> m_allTimes;
    QHash<QDateTime, Coordinates> m_coordinates;

    int m_exactMatchTolerance;
    int m_maximumInterpolationInterval;
    int m_maximumInterpolationDistance;

    QImage m_timezoneMap;
    QJsonObject m_timezoneMapping;
    QByteArray m_lastDetectedTimeZoneId;

};

#endif // GPXENGINE_H
