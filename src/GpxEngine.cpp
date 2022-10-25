// SPDX-FileCopyrightText: 2020-2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes

#include "GpxEngine.h"
#include "GeoDataModel.h"
#include "Logging.h"

#include "debugMode.h"

// Marble includes
#include <marble/GeoDataCoordinates.h>

// Qt includes

#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QFile>
#include <QLoggingCategory>
#include <QTimeZone>

// C++ includes
#include <cmath>

static const auto s_gpx    = QStringLiteral("gpx");
static const auto s_trk    = QStringLiteral("trk");
static const auto s_trkpt  = QStringLiteral("trkpt");
static const auto s_lon    = QStringLiteral("lon");
static const auto s_lat    = QStringLiteral("lat");
static const auto s_ele    = QStringLiteral("ele");
static const auto s_time   = QStringLiteral("time");
static const auto s_trkseg = QStringLiteral("trkseg");

GpxEngine::GpxEngine(QObject *parent, GeoDataModel *geoDataModel)
    : QObject(parent),
      m_geoDataModel(geoDataModel)
{
    // Load the timezone map image
    const auto timezoneMapFile = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                        QStringLiteral("timezones.png"));
    if (! timezoneMapFile.isEmpty()) {
        m_timezoneMap = QImage(timezoneMapFile);
        if (! m_timezoneMap.isNull()) {
            m_timezoneMapWidth = m_timezoneMap.size().width();
            m_timezoneMapHeight = m_timezoneMap.size().height();
        }
        qCDebug(KGeoTagLog) << "Loaded the timezone data map";
    } else {
        // This should not happen
        qCDebug(KGeoTagLog) << "Failed to load the timezone data map!";
    }

    // Load the color-timezone mapping
    const auto timezoneMappingFile = QStandardPaths::locate(QStandardPaths::AppDataLocation,
                                                            QStringLiteral("timezones.json"));
    if (! timezoneMappingFile.isEmpty()) {
        QFile jsonData(timezoneMappingFile);
        if (jsonData.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const auto jsonDocument = QJsonDocument::fromJson(jsonData.readAll());
            jsonData.close();
            m_timezoneMapping = jsonDocument.object();
        }
    }

    // Check if all listed timezones are valid

    if (! m_timezoneMapping.isEmpty()) {
        const auto allTimeZones = QTimeZone::availableTimeZoneIds();
        const auto keys = m_timezoneMapping.keys();

        QVector<QByteArray> invalidIds;

        for (const auto &key : keys) {
            const auto timeZoneId = m_timezoneMapping.value(key).toString().toUtf8();
            if (! allTimeZones.contains(timeZoneId)) {
                // FIXME: This should be a temporary workaround!
                // Starting with IANA's timezone data 2022c, "Europe/Kiev" is now called
                // "Europe/Kyiv". As soon as this version is included in e.g. Ubuntu LTS and such,
                // this can be removed.
                if (timeZoneId == QStringLiteral("Europe/Kyiv").toUtf8()
                    && allTimeZones.contains(QStringLiteral("Europe/Kiev").toUtf8())) {

                    m_timezoneMapping[key] = QJsonValue(QStringLiteral("Europe/Kiev"));
                    qCDebug(KGeoTagLog) << "Mapped timezone \"Europe/Kyiv\" to \"Europe/Kiev\"";
                    continue;
                }

                invalidIds.append(timeZoneId);
            }
        }

        qCDebug(KGeoTagLog) << "Loaded" << m_timezoneMapping.count() << "timezone IDs";

        if (invalidIds.count() > 0) {
            qCWarning(KGeoTagLog) << "Found" << invalidIds.count() << "unusable timezone ID(s)!";
            qCWarning(KGeoTagLog) << "    The following IDs are not represented in "
                                  << "QTimeZone::availableTimeZoneIds():";
            for (const auto &id : invalidIds) {
                qCWarning(KGeoTagLog) << "   " << id;
            }
        }

    } else {
        // This should not happen
        qCWarning(KGeoTagLog) << "Could not load any timezone IDs!";
    }
}

GpxEngine::LoadInfo GpxEngine::load(const QString &path)
{
    if (m_geoDataModel->contains(path)) {
        return { LoadResult::AlreadyLoaded };
    }

    QFile gpxFile(path);

    if (! gpxFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return { LoadResult::OpenFailed };
    }

    QXmlStreamReader xml(&gpxFile);

    double lon = 0.0;
    double lat = 0.0;
    double alt = 0.0;
    QDateTime time;

    QVector<QDateTime> segmentTimes;
    QVector<Coordinates> segmentCoordinates;

    QVector<QVector<Coordinates>> allSegments;
    QVector<QVector<QDateTime>> allSegmentTimes;

    bool gpxFound = false;
    bool trackStartFound = false;

    int tracks = 0;
    int segments = 0;
    int points = 0;

    while (! xml.atEnd()) {
        if (xml.hasError()) {
            return { LoadResult::XmlError };
        }

        const QXmlStreamReader::TokenType token = xml.readNext();
        const QStringRef name = xml.name();

        if (token == QXmlStreamReader::StartElement) {
            if (! gpxFound) {
                if (name != s_gpx) {
                    continue;
                } else {
                    gpxFound = true;
                }
            }

            if (! trackStartFound) {
                if (name != s_trk) {
                    continue;
                } else {
                    trackStartFound = true;
                    tracks++;
                }
            }

            if (name == s_trkseg) {
                segments++;

            } else if (name == s_trkpt) {
                QXmlStreamAttributes attributes = xml.attributes();
                lon = attributes.value(s_lon).toDouble();
                lat = attributes.value(s_lat).toDouble();
                points++;

            } else if (name == s_ele) {
                xml.readNext();
                alt = xml.text().toDouble();

            } else if (name == s_time) {
                xml.readNext();
                time = QDateTime::fromString(xml.text().toString(), Qt::ISODate);

                // Strip out milliseconds if the GPX provides them to allow seconds-exact matching
                const auto msec = time.time().msec();
                if (msec != 0) {
                    time = time.addMSecs(msec * -1);
                }
            }

        } else if (token == QXmlStreamReader::EndElement) {
            if (name == s_trkpt) {
                segmentTimes.append(time);
                segmentCoordinates.append(Coordinates(lon, lat, alt, true));
                alt = 0.0;
                time = QDateTime();

            } else if (name == s_trkseg && ! segmentCoordinates.isEmpty()) {
                allSegmentTimes.append(segmentTimes);
                allSegments.append(segmentCoordinates);
                segmentTimes.clear();
                segmentCoordinates.clear();

            } else if (name == s_trk) {
                trackStartFound = false;
            }
        }
    }

    if (! gpxFound) {
        return { LoadResult::NoGpxElement, tracks, segments, points };
    }

    if (points == 0) {
        return { LoadResult::NoGeoData, tracks, segments, points };
    }

    // All okay :-)

    // Pass the loaded data to the GeoDataModel
    m_geoDataModel->addTrack(path, allSegmentTimes, allSegments);

    // Detect the presumable timezone the corresponding photos were taken in

    // Get the loaded path's bounding box's center point
    const auto trackCenter = m_geoDataModel->trackBoxCenter(path);

    // Scale the coordinates to the image size, relative to the image center
    int mappedLon = std::round(trackCenter.lon() / 180.0 * (m_timezoneMapWidth / 2.0));
    int mappedLat = std::round(trackCenter.lat() / 90.0 * (m_timezoneMapHeight / 2.0));

    // Move the mapped coordinates to the left lower edge
    mappedLon = m_timezoneMapWidth / 2 + mappedLon;
    mappedLat = m_timezoneMapHeight - (m_timezoneMapHeight / 2 + mappedLat);

    // Get the respective pixel's color
    const auto timezoneColor = m_timezoneMap.pixelColor(mappedLon, mappedLat).name();

    // Lookup the corresponding timezone
    const auto timezoneId = m_timezoneMapping.value(timezoneColor);
    if (timezoneId.isString()) {
        m_lastDetectedTimeZoneId = timezoneId.toString().toUtf8();
    } else {
        m_lastDetectedTimeZoneId.clear();
    }

    return { LoadResult::Okay, tracks, segments, points };
}

void GpxEngine::setMatchParameters(int exactMatchTolerance, int maximumInterpolationInterval,
                                   int maximumInterpolationDistance)
{
    m_exactMatchTolerance = exactMatchTolerance;
    m_maximumInterpolationInterval = maximumInterpolationInterval;
    m_maximumInterpolationDistance = maximumInterpolationDistance;
}

Coordinates GpxEngine::findExactCoordinates(const QDateTime &time, int deviation) const
{
    if (deviation == 0) {
        return findExactCoordinates(time);
    } else {
        const QDateTime fixedTime = time.addSecs(deviation);
        return findExactCoordinates(fixedTime);
    }
}

Coordinates GpxEngine::findExactCoordinates(const QDateTime &time) const
{
    // Iterate over all loaded files we have
    for (const auto &trackPoints : m_geoDataModel->trackPoints()) {
        // Check for an exact match
        if (trackPoints.contains(time)) {
            return trackPoints.value(time);
        }

        // Check for a match with +/- the maximum tolerable deviation
        for (int i = 1; i <= m_exactMatchTolerance; i++) {
            const auto timeBefore = time.addSecs(i * -1);
            if (trackPoints.contains(timeBefore)) {
                return trackPoints.value(timeBefore);
            }
            const auto timeAfter = time.addSecs(i);
            if (trackPoints.contains(timeAfter)) {
                return trackPoints.value(timeAfter);
            }
        }
    }

    // No match found
    return Coordinates();
}

Coordinates GpxEngine::findInterpolatedCoordinates(const QDateTime &time, int deviation) const
{
    if (deviation == 0) {
        return findInterpolatedCoordinates(time);
    } else {
        const QDateTime fixedTime = time.addSecs(deviation);
        return findInterpolatedCoordinates(fixedTime);
    }
}

Coordinates GpxEngine::findInterpolatedCoordinates(const QDateTime &time) const
{
    // Iterate over all loaded files we have
    for (int i = 0; i < m_geoDataModel->dateTimes().count(); i++) {
        const auto &dateTimes = m_geoDataModel->dateTimes().at(i);
        const auto &trackPoints = m_geoDataModel->trackPoints().at(i);

        // This only works if we at least have at least 2 points ;-)
        if (dateTimes.count() < 2) {
            continue;
        }

        // If the image's date is before the first or after the last point we have,
        // it can't be assigned.
        if (time < dateTimes.first() || time > dateTimes.last()) {
            continue;
        }

        // Check for an exact match (without tolerance)
        // This also eliminates the case that the time could be the first one.
        // We thus can be sure the first entry in dateTimes is earlier than the time requested.
        if (dateTimes.contains(time)) {
            return trackPoints.value(time);
        }

        // Search for the first time earlier than the image's

        int start = 0;
        int end = dateTimes.count() - 1;
        int index = 0;
        int lastIndex = -1;

        while (true) {
            index = start + (end - start) / 2;
            if (index == lastIndex) {
                break;
            }

            if (dateTimes.at(index) > time) {
                end = index;
            } else {
                start = index;
            }

            lastIndex = index;
        }

        // If the found point is the last one, we can't interpolate and use it directly
        const auto &closestBefore = dateTimes.at(index);
        if (closestBefore == dateTimes.last()) {
            return trackPoints.value(closestBefore);
        }

        // Interpolate between the two coordinates

        const auto &closestAfter = dateTimes.at(index + 1);

        // Check for a maximum time interval between the points if requested
        if (m_maximumInterpolationInterval != -1
            && closestBefore.secsTo(closestAfter) > m_maximumInterpolationInterval) {

            continue;
        }

        // Create Marble coordinates from the cache for further calculations
        const auto &pointBefore = trackPoints[closestBefore];
        const auto &pointAfter = trackPoints[closestAfter];
        const auto coordinatesBefore = Marble::GeoDataCoordinates(
            pointBefore.lon(), pointBefore.lat(), pointBefore.alt(),
            Marble::GeoDataCoordinates::Degree);
        const auto coordinatesAfter = Marble::GeoDataCoordinates(
            pointAfter.lon(), pointAfter.lat(), pointAfter.alt(),
            Marble::GeoDataCoordinates::Degree);

        // Check for a maximum distance between the points if requested

        if (m_maximumInterpolationDistance != -1
            && coordinatesBefore.sphericalDistanceTo(coordinatesAfter) * KGeoTag::earthRadius
            > m_maximumInterpolationDistance) {

            continue;
        }

        // Calculate an interpolated position between the coordinates

        const int secondsBefore = closestBefore.secsTo(time);
        const double fraction = double(secondsBefore)
                                / double(secondsBefore + time.secsTo(closestAfter));
        const auto interpolated = coordinatesBefore.interpolate(coordinatesAfter, fraction);

        return Coordinates(interpolated.longitude(Marble::GeoDataCoordinates::Degree),
                        interpolated.latitude(Marble::GeoDataCoordinates::Degree),
                        interpolated.altitude(),
                        true);
    }

    // No match found
    return Coordinates();
}

QByteArray GpxEngine::lastDetectedTimeZoneId() const
{
    return m_lastDetectedTimeZoneId;
}

bool GpxEngine::timeZoneDataLoaded() const
{
    return ! m_timezoneMap.isNull() && ! m_timezoneMapping.isEmpty();
}
