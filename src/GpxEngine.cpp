/* Copyright (c) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Local includes
#include "GpxEngine.h"
#include "Settings.h"

// Marble includes
#include <marble/GeoDataCoordinates.h>

// Qt includes
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

GpxEngine::GpxEngine(QObject *parent, Settings *settings)
    : QObject(parent), m_settings(settings)
{
}

GpxEngine::LoadResult GpxEngine::load(const QString &path)
{
    QFile gpxFile(path);

    if (! gpxFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return LoadResult::OpenFailed;
    }

    QXmlStreamReader xml(&gpxFile);

    double lon;
    double lat;
    QDateTime time;

    QVector<QDateTime> segmentTimes;
    QVector<KGeoTag::Coordinates> segmentCoordinates;

    bool trackFound = false;
    while (! xml.atEnd()) {
        if (xml.hasError()) {
            return LoadResult::XmlError;
        }

        const QXmlStreamReader::TokenType token = xml.readNext();
        const QStringRef name = xml.name();

        if (! trackFound && name != QStringLiteral("trk")) {
            continue;
        } else {
            trackFound = true;
        }

        if (token == QXmlStreamReader::StartElement) {
            if (name == QStringLiteral("trkpt")) {
                QXmlStreamAttributes attributes = xml.attributes();
                lon = attributes.value(QStringLiteral("lon")).toDouble();
                lat = attributes.value(QStringLiteral("lat")).toDouble();
            } else if (name == QStringLiteral("time")) {
                xml.readNext();
                time = QDateTime::fromString(xml.text().toString(), Qt::ISODate);
            }

        } else if (token == QXmlStreamReader::EndElement) {
            if (name == QStringLiteral("time")) {
                segmentTimes.append(time);
                segmentCoordinates.append({ lon, lat });
            } else if (name == QStringLiteral("trkseg") && ! segmentCoordinates.isEmpty()) {
                emit segmentLoaded(segmentCoordinates);
                for (int i = 0; i < segmentTimes.count(); i++) {
                    const auto &time = segmentTimes.at(i);
                    m_allTimes.append(time);
                    m_coordinates[time] = segmentCoordinates.at(i);
                }
                segmentTimes.clear();
                segmentCoordinates.clear();
            }
        }
    }

    std::sort(m_allTimes.begin(), m_allTimes.end());

    return LoadResult::Okay;
}

KGeoTag::Coordinates GpxEngine::findExactCoordinates(const QDateTime &time, int deviation) const
{
    if (deviation == 0) {
        return findExactCoordinates(time);
    } else {
        const QDateTime fixedTime = time.addSecs(deviation);
        return findExactCoordinates(fixedTime);
    }
}

KGeoTag::Coordinates GpxEngine::findExactCoordinates(const QDateTime &time) const
{
    // Check for an exact match
    if (m_coordinates.contains(time)) {
        return m_coordinates.value(time);
    }

    // Check for a match with +/- the maximum tolerable deviation
    for (int i = 1; i <= m_settings->exactMatchTolerance(); i++) {
        const auto timeBefore = time.addSecs(i * -1);
        if (m_coordinates.contains(timeBefore)) {
            return m_coordinates.value(timeBefore);
        }
        const auto timeAfter = time.addSecs(i);
        if (m_coordinates.contains(timeAfter)) {
            return m_coordinates.value(timeAfter);
        }
    }

    // No match found
    return KGeoTag::NoCoordinates;
}

KGeoTag::Coordinates GpxEngine::findInterpolatedCoordinates(const QDateTime &time,
                                                            int deviation) const
{
    if (deviation == 0) {
        return findInterpolatedCoordinates(time);
    } else {
        const QDateTime fixedTime = time.addSecs(deviation);
        return findInterpolatedCoordinates(fixedTime);
    }
}

KGeoTag::Coordinates GpxEngine::findInterpolatedCoordinates(const QDateTime &time) const
{
    // If the image's date is before the first or after the last point we have,
    // it can't be assigned.
    if (time < m_allTimes.first() || time > m_allTimes.last()) {
        return KGeoTag::NoCoordinates;
    }

    // Check for an exact match (without tolerance)
    // This also eliminates the case that the time could be the first one.
    // We thus can be sure the first entry in m_allTimes is earlier than the time requested.
    if (m_allTimes.contains(time)) {
        return m_coordinates.value(time);
    }

    // Search for the first time earlier than the image's

    int start = 0;
    int end = m_allTimes.count() - 1;
    int index = 0;
    int lastIndex = -1;

    while (true) {
        index = start + (end - start) / 2;
        if (index == lastIndex) {
            break;
        }

        if (m_allTimes.at(index) > time) {
            end = index;
        } else {
            start = index;
        }

        lastIndex = index;
    }

    // If the found point is the last one, we can't interpolate and use it directly
    const auto &closestBefore = m_allTimes.at(index);
    if (closestBefore == m_allTimes.last()) {
        return m_coordinates.value(closestBefore);
    }

    // Interpolate between the two coordinates

    const auto &closestAfter = m_allTimes.at(index + 1);
    const int maximumInterval = m_settings->maximumInterpolationInterval();
    if (maximumInterval != -1 && closestBefore.secsTo(closestAfter) > maximumInterval) {
        return KGeoTag::NoCoordinates;
    }

    const auto &pointBefore = m_coordinates[closestBefore];
    const auto &pointAfter = m_coordinates[closestAfter];
    const auto coordinatesBefore = Marble::GeoDataCoordinates(
        pointBefore.lon, pointBefore.lat, 0.0, Marble::GeoDataCoordinates::Degree);
    const auto coordinatesAfter = Marble::GeoDataCoordinates(
        pointAfter.lon, pointAfter.lat, 0.0, Marble::GeoDataCoordinates::Degree);

    const int secondsBefore = closestBefore.secsTo(time);
    const double fraction = double(secondsBefore)
                            / double(secondsBefore + time.secsTo(closestAfter));
    const auto interpolated = coordinatesBefore.interpolate(coordinatesAfter, fraction);

    return KGeoTag::Coordinates { interpolated.longitude(Marble::GeoDataCoordinates::Degree),
                                  interpolated.latitude(Marble::GeoDataCoordinates::Degree) };
}
