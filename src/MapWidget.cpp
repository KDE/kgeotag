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
#include "MapWidget.h"
#include "Settings.h"
#include "ImageCache.h"
#include "Coordinates.h"

// Marble includes
#include <marble/GeoPainter.h>
#include <marble/AbstractFloatItem.h>
#include <marble/MarbleModel.h>
#include <marble/GeoDataLatLonAltBox.h>

// Qt includes
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QXmlStreamReader>

// C++ includes
#include <algorithm>

MapWidget::MapWidget(Settings *settings, ImageCache *imageCache, QWidget *parent)
    : Marble::MarbleWidget(parent), m_settings(settings), m_imageCache(imageCache)
{
    setAcceptDrops(true);

    setProjection(Marble::Mercator);
    setMapThemeId(QStringLiteral("earth/openstreetmap/openstreetmap.dgml"));

    m_trackPen.setColor(QColor(255, 0, 255, 150));
    m_trackPen.setWidth(3);
    m_trackPen.setStyle(Qt::DotLine);
    m_trackPen.setCapStyle(Qt::RoundCap);
    m_trackPen.setJoinStyle(Qt::RoundJoin);
}

void MapWidget::addGpx(const QString &path)
{
    QFile gpxFile(path);
    gpxFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(&gpxFile);

    Marble::GeoDataLineString lineString;
    Marble::GeoDataLatLonAltBox trackBox;
    double lon;
    double lat;
    QDateTime time;

    bool trackFound = false;
    while (! xml.atEnd() && ! xml.hasError()) {
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
                const Marble::GeoDataCoordinates coordinates
                    = Marble::GeoDataCoordinates(lon, lat, 0.0, Marble::GeoDataCoordinates::Degree);
                lineString.append(coordinates);

            } else if (name == QStringLiteral("time")) {
                xml.readNext();
                time = QDateTime::fromString(xml.text().toString(), Qt::ISODate);
            }

        } else if (token == QXmlStreamReader::EndElement) {
            if (name == QStringLiteral("time")) {
                m_points[time] = Coordinates::Data { lon, lat };
                m_allTimes.append(time);

            } else if (name == QStringLiteral("trkseg") && ! lineString.isEmpty()) {
                m_tracks.append(lineString);

                const auto &box = lineString.latLonAltBox();
                if (trackBox.isEmpty()) {
                    trackBox = box;
                } else {
                    trackBox |= box;
                }

                lineString.clear();
            }
        }
    }

    std::sort(m_allTimes.begin(), m_allTimes.end());
    centerOn(trackBox);
}

void MapWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("text/plain"))
        && m_imageCache->contains(event->mimeData()->text())) {

        event->acceptProposedAction();
    }
}

void MapWidget::dropEvent(QDropEvent *event)
{
    const auto dropPosition = event->pos();

    qreal lon;
    qreal lat;
    if (! geoCoordinates(dropPosition.x(), dropPosition.y(), lon, lat,
                         Marble::GeoDataCoordinates::Degree)) {
        return;
    }

    const QString path = event->mimeData()->text();
    addImage(path, lon, lat);
    reloadMap();

    m_imageCache->setCoordinates(path, lon, lat);
    emit imageAssigned(path);

    event->acceptProposedAction();
}

void MapWidget::addImage(const QString &path, const Coordinates::Data &coordinates)
{
    addImage(path, coordinates.lon, coordinates.lat);
}

void MapWidget::addImage(const QString &path, double lon, double lat)
{
    m_images[path] = Marble::GeoDataCoordinates(lon, lat, 0.0, Marble::GeoDataCoordinates::Degree);
}

void MapWidget::customPaint(Marble::GeoPainter *painter)
{
    const auto images = m_images.keys();
    for (const auto image : images) {
        painter->drawPixmap(m_images.value(image),
                            QPixmap::fromImage(m_imageCache->thumbnail(image)));
    }

    painter->setPen(m_trackPen);
    for (const auto &lineString : m_tracks) {
        painter->drawPolyline(lineString);
    }
}

void MapWidget::saveSettings()
{
    // Save the floaters visibility

    QHash<QString, bool> visibility;

    const auto floatItemsList = floatItems();
    for (const auto &item : floatItemsList) {
        visibility.insert(item->name(), item->visible());
    }

    m_settings->saveFloatersVisibility(visibility);

    // Save the current center point
    const auto center = focusPoint();
    m_settings->saveMapCenter(Coordinates::Data {
                                  center.longitude(Marble::GeoDataCoordinates::Degree),
                                  center.latitude(Marble::GeoDataCoordinates::Degree) });

    // Save the zoom level
    m_settings->saveZoom(zoom());
}

void MapWidget::restoreSettings()
{
    // Restore the floaters visiblility
    const auto floatersVisiblility = m_settings->floatersVisibility();
    const auto floatItemsList = floatItems();
    for (const auto &item : floatItemsList) {
        const auto name = item->name();
        if (floatersVisiblility.contains(name)) {
            item->setVisible(floatersVisiblility.value(name));
        }
    }

    // Restore map's last center point
    const auto [ lon, lat, isSet ] = m_settings->mapCenter();
    centerOn(lon, lat);

    // Restore the last zoom level
    setZoom(m_settings->zoom());
}

void MapWidget::centerImage(const QString &path)
{
    const auto coordinates = m_imageCache->coordinates(path);
    centerOn(coordinates.lon, coordinates.lat);
}

Coordinates::Data MapWidget::findExactCoordinates(const QDateTime &time) const
{
    // Check for an exact match
    if (m_points.contains(time)) {
        return m_points.value(time);
    }

    // Check for a match with +/- the secondsTolerance
    for (int i = 1; i <= m_settings->secondsTolerance(); i++) {
        const auto timeBefore = time.addSecs(i * -1);
        if (m_points.contains(timeBefore)) {
            return m_points.value(timeBefore);
        }
        const auto timeAfter = time.addSecs(i);
        if (m_points.contains(timeAfter)) {
            return m_points.value(timeAfter);
        }
    }

    // No match found
    return Coordinates::Data { 0.0, 0.0, false };
}

Coordinates::Data MapWidget::findInterpolatedCoordinates(const QDateTime &time) const
{
    // If the image's date is before the first or after the last point we have,
    // it can't be assigned.
    if (time < m_allTimes.first() || time > m_allTimes.last()) {
        return Coordinates::Data { 0.0, 0.0, false };
    }

    // Check for an exact match (without tolerance)
    // This also eliminates the case that the time could be the first one.
    // We thus can be sure the first entry in m_allTimes is earlier than the time requested.
    if (m_allTimes.contains(time)) {
        return m_points.value(time);
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
        return m_points.value(closestBefore);
    }


    // Interpolate between the two coordinates

    const auto &closestAfter = m_allTimes.at(index + 1);
    const auto &pointBefore = m_points[closestBefore];
    const auto &pointAfter = m_points[closestAfter];
    const auto coordinatesBefore = Marble::GeoDataCoordinates(
        pointBefore.lon, pointBefore.lat, 0.0, Marble::GeoDataCoordinates::Degree);
    const auto coordinatesAfter = Marble::GeoDataCoordinates(
        pointAfter.lon, pointAfter.lat, 0.0, Marble::GeoDataCoordinates::Degree);

    const int secondsBefore = closestBefore.secsTo(time);
    const double fraction = double(secondsBefore)
                            / double(secondsBefore + time.secsTo(closestAfter));
    const auto interpolated = coordinatesBefore.interpolate(coordinatesAfter, fraction);

    return Coordinates::Data { interpolated.longitude(Marble::GeoDataCoordinates::Degree),
                               interpolated.latitude(Marble::GeoDataCoordinates::Degree) };
}
