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
#include "Settings.h"

// Qt includes
#include <QDebug>
#include <QSize>

namespace
{

const QLatin1String c_main("main/");
const QString c_main_windowGeometry = c_main + QLatin1String("window_geometry");
const QString c_main_windowState   = c_main + QLatin1String("window_state");
const QString c_main_lastOpenPath = c_main + QLatin1String("last_open_path");

const QLatin1String c_floatersVisibility("floaters_visibility/");

const QLatin1String c_map("map/");
const QString c_map_centerLon = c_map + QLatin1String("center_lon");
const QString c_map_centerLat = c_map + QLatin1String("center_lat");
const QString c_map_zoom = c_map + QLatin1String("zoom");

const QLatin1String c_tracks("tracks/");
const QString c_tracks_color = c_tracks + QLatin1String("color");

}

Settings::Settings(QObject *parent)
    : QSettings(QStringLiteral("kgeotag"), QStringLiteral("kgeotag"), parent)
{
}

void Settings::saveMainWindowGeometry(const QByteArray &data)
{
    setValue(c_main_windowGeometry, data);
}

QByteArray Settings::mainWindowGeometry() const
{
    return value(c_main_windowGeometry, QByteArray()).toByteArray();
}

void Settings::saveMainWindowState(const QByteArray &data)
{
    setValue(c_main_windowState, data);
}

QByteArray Settings::mainWindowState() const
{
    return value(c_main_windowState, QByteArray()).toByteArray();
}

void Settings::saveFloatersVisibility(const QHash<QString, bool> &data)
{
    const auto keys = data.keys();
    for (const auto &key : keys) {
        setValue(c_floatersVisibility + key, data.value(key));
    }
}

QHash<QString, bool> Settings::floatersVisibility()
{
    QHash<QString, bool> data;

    beginGroup(c_floatersVisibility);
    const auto keys = allKeys();
    endGroup();

    for (const auto &key : keys) {
        data.insert(key, value(c_floatersVisibility + key).toBool());
    }

    return data;
}

void Settings::saveMapCenter(const KGeoTag::Coordinates &coordinates)
{
    setValue(c_map_centerLon, coordinates.lon);
    setValue(c_map_centerLat, coordinates.lat);
}

KGeoTag::Coordinates Settings::mapCenter() const
{
    return KGeoTag::Coordinates { value(c_map_centerLon, 0).toDouble(),
                                  value(c_map_centerLat, 0).toDouble() };
}

void Settings::saveZoom(int zoom)
{
    setValue(c_map_zoom, zoom);
}

int Settings::zoom() const
{
    return value(c_map_zoom, 1520).toInt();
}

void Settings::saveLastOpenPath(const QString &path)
{
    setValue(c_main_lastOpenPath, path);
}

QString Settings::lastOpenPath() const
{
    return value(c_main_lastOpenPath, QString()).toString();
}

QSize Settings::thumbnailSize() const
{
    return QSize(32, 32);
}

QSize Settings::previewSize() const
{
    return QSize(400, 400);
}

int Settings::secondsTolerance() const
{
    return 10;
}

void Settings::saveTrackColor(const QColor &color)
{
    setValue(c_tracks_color, color);
}

QColor Settings::trackColor() const
{
    QColor color = value(c_tracks_color).value<QColor>();
    if (! color.isValid()) {
        color = QColor(255, 0, 255, 150);
    }

    return color;
}
