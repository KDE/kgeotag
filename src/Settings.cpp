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
#include "Settings.h"

// Qt includes
#include <QDebug>
#include <QSize>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

// Main

static const QLatin1String s_main("main/");
static const QString s_main_windowGeometry = s_main + QLatin1String("window_geometry");
static const QString s_main_windowState = s_main + QLatin1String("window_state");
static const QString s_main_lastOpenPath = s_main + QLatin1String("last_open_path");
static const QString s_main_spitImagesList = s_main + QLatin1String("split_images_list");

// Map

static const QLatin1String s_map("map/");
static const QString s_map_showCrosshairs = s_map + QLatin1String("show_crosshairs");
static const QString s_map_centerLon = s_map + QLatin1String("center_lon");
static const QString s_map_centerLat = s_map + QLatin1String("center_lat");
static const QString s_map_zoom = s_map + QLatin1String("zoom");

static const QLatin1String s_floatersVisibility("floaters_visibility/");

// Tracks

static const QLatin1String s_track("track/");
static const QString s_track_color = s_track + QLatin1String("color");
static const QString s_track_width = s_track + QLatin1String("width");
static const QString s_track_style = s_track + QLatin1String("style");
static const QVector<Qt::PenStyle> s_track_styleEnum {
    Qt::SolidLine,
    Qt::DashLine,
    Qt::DotLine,
    Qt::DashDotLine,
    Qt::DashDotDotLine
};
static const QVector<QString> s_track_styleString {
    QStringLiteral("solid"),
    QStringLiteral("dashes"),
    QStringLiteral("dots"),
    QStringLiteral("dash_dot"),
    QStringLiteral("dash_dot_dot")
};

// Images

static const QLatin1String s_images("images/");
static const QString s_images_thumnailSize = s_images + QLatin1String("thumbnail_size");
static const QString s_images_previewSize = s_images + QLatin1String("preview_size");

// Assignment

static const QLatin1String s_assignment("assignment/");
static const QString s_assignment_exactMatchTolerance
    = s_assignment + QLatin1String("exact_match_tolerance");
static const QString s_assignment_maximumInterpolationInterval
    = s_assignment + QLatin1String("maximum_interpolation_interval");
static const QString s_assignment_maximumInterpolationDistance
    = s_assignment + QLatin1String("maximum_interpolation_distance");

// Elevation lookup

static const QLatin1String s_elevationLookup("elevation_lookup/");

static const QString s_elevationLookup_enabled = s_elevationLookup + QLatin1String("enabled");

static const QVector<QString> s_elevationDatasets = {
    QStringLiteral("aster30m"),
    QStringLiteral("etopo1"),
    QStringLiteral("eudem25m"),
    QStringLiteral("mapzen"),
    QStringLiteral("ned10m"),
    QStringLiteral("nzdem8m"),
    QStringLiteral("srtm30m"),
    QStringLiteral("srtm90m"),
    QStringLiteral("emod2018"),
    QStringLiteral("gebco2020")
};
static const QString &s_defaultElevationDataset = s_elevationDatasets.at(0);
static const QString s_elevationLookup_dataset = s_elevationLookup + QLatin1String("dataset");

// Saving

static const QLatin1String s_save("save/");

static const QVector<QString> s_writeModes = {
    QStringLiteral("WRITETOIMAGEONLY"),
    QStringLiteral("WRITETOSIDECARONLY"),
    QStringLiteral("WRITETOSIDECARANDIMAGE")
};
static const QString &s_defaultWriteMode = s_writeModes.at(0);
static const QString s_save_writeMode = s_save + QLatin1String("write_mode");

static const QString s_save_createBackups = s_save + QLatin1String("create_backups");

// Bookmarks

static const QLatin1String s_bookmarks("bookmarks/");
static const QString s_bookmarks_version = s_bookmarks + QLatin1String("version");
static const QString s_bookmarks_data = s_bookmarks + QLatin1String("data");

static constexpr int s_bookmarksDataVersion = 1;
static const QString s_bookmarksDataLabel = QStringLiteral("label");
static const QString s_bookmarksDataLon = QStringLiteral("lon");
static const QString s_bookmarksDataLat = QStringLiteral("lat");
static const QString s_bookmarksDataAlt = QStringLiteral("alt");

Settings::Settings(QObject *parent)
    : QSettings(QStringLiteral("kgeotag"), QStringLiteral("kgeotag"), parent)
{
}

void Settings::saveMainWindowGeometry(const QByteArray &data)
{
    setValue(s_main_windowGeometry, data);
}

QByteArray Settings::mainWindowGeometry() const
{
    return value(s_main_windowGeometry, QByteArray()).toByteArray();
}

void Settings::saveMainWindowState(const QByteArray &data)
{
    setValue(s_main_windowState, data);
}

QByteArray Settings::mainWindowState() const
{
    return value(s_main_windowState, QByteArray()).toByteArray();
}

void Settings::saveShowCrosshairs(bool state)
{
    setValue(s_map_showCrosshairs, state);
}

bool Settings::showCrosshairs() const
{
    return value(s_map_showCrosshairs, true).toBool();
}

void Settings::saveFloatersVisibility(const QHash<QString, bool> &data)
{
    const auto keys = data.keys();
    for (const auto &key : keys) {
        setValue(s_floatersVisibility + key, data.value(key));
    }
}

QHash<QString, bool> Settings::floatersVisibility()
{
    QHash<QString, bool> data;

    beginGroup(s_floatersVisibility);
    const auto keys = allKeys();
    endGroup();

    for (const auto &key : keys) {
        data.insert(key, value(s_floatersVisibility + key).toBool());
    }

    return data;
}

void Settings::saveMapCenter(const Coordinates &coordinates)
{
    setValue(s_map_centerLon, coordinates.lon());
    setValue(s_map_centerLat, coordinates.lat());
}

Coordinates Settings::mapCenter() const
{
    return Coordinates(value(s_map_centerLon, 0).toDouble(),
                       value(s_map_centerLat, 0).toDouble(),
                       0.0,
                       true);
}

void Settings::saveZoom(int zoom)
{
    setValue(s_map_zoom, zoom);
}

int Settings::zoom() const
{
    return value(s_map_zoom, 1520).toInt();
}

void Settings::saveLastOpenPath(const QString &path)
{
    setValue(s_main_lastOpenPath, path);
}

QString Settings::lastOpenPath() const
{
    return value(s_main_lastOpenPath, QString()).toString();
}

void Settings::saveThumbnailSize(int size)
{
    setValue(s_images_thumnailSize, size);
}

QSize Settings::thumbnailSize() const
{
    const int size = value(s_images_thumnailSize, 32).toInt();
    return QSize(size, size);
}

void Settings::savePreviewSize(int size)
{
    setValue(s_images_previewSize, size);
}

QSize Settings::previewSize() const
{
    const int size = value(s_images_previewSize, 400).toInt();
    return QSize(size, size);
}

void Settings::saveExactMatchTolerance(int seconds)
{
    setValue(s_assignment_exactMatchTolerance, seconds);
}

int Settings::exactMatchTolerance() const
{
    return value(s_assignment_exactMatchTolerance, 10).toInt();
}

void Settings::saveMaximumInterpolationInterval(int seconds)
{
    setValue(s_assignment_maximumInterpolationInterval, seconds);
}

int Settings::maximumInterpolationInterval() const
{
    return value(s_assignment_maximumInterpolationInterval, -1).toInt();
}

void Settings::saveLookupElevation(bool state)
{
    setValue(s_elevationLookup_enabled, state);
}

bool Settings::lookupElevation() const
{
    return value(s_elevationLookup_enabled, false).toBool();
}

void Settings::saveElevationDataset(const QString &id)
{
    setValue(s_elevationLookup_dataset, id);
}

QString Settings::elevationDataset() const
{
    const auto dataset = value(s_elevationLookup_dataset, s_defaultElevationDataset).toString();
    return s_elevationDatasets.contains(dataset) ? dataset : s_defaultElevationDataset;
}

void Settings::saveMaximumInterpolationDistance(int meters)
{
    setValue(s_assignment_maximumInterpolationDistance, meters);
}

int Settings::maximumInterpolationDistance() const
{
    return value(s_assignment_maximumInterpolationDistance, -1).toInt();
}

void Settings::saveTrackColor(const QColor &color)
{
    setValue(s_track_color, color);
}

QColor Settings::trackColor() const
{
    QColor color = value(s_track_color).value<QColor>();
    if (! color.isValid()) {
        color = QColor(255, 0, 255, 150);
    }

    return color;
}

void Settings::saveTrackStyle(Qt::PenStyle style)
{
    setValue(s_track_style, s_track_styleString.at(s_track_styleEnum.indexOf(style)));
}

Qt::PenStyle Settings::trackStyle() const
{
    QString styleString = value(s_track_style).toString();
    if (s_track_styleString.contains(styleString)) {
        return s_track_styleEnum.at(s_track_styleString.indexOf(styleString));
    } else {
        return Qt::DotLine;
    }
}

void Settings::saveTrackWidth(int width)
{
    setValue(s_track_width, width);
}

int Settings::trackWidth() const
{
    return value(s_track_width, 3).toInt();
}

void Settings::saveCreateBackups(bool state)
{
    setValue(s_save_createBackups, state);
}

bool Settings::createBackups() const
{
    return value(s_save_createBackups, true).toBool();
}

void Settings::saveWriteMode(const QString &writeMode)
{
    setValue(s_save_writeMode, writeMode);
}

QString Settings::writeMode() const
{
    const auto mode = value(s_save_writeMode, s_defaultWriteMode).toString();
    return s_writeModes.contains(mode) ? mode : s_defaultWriteMode;
}

void Settings::saveBookmarks(const QHash<QString, Coordinates> *bookmarks)
{
    QJsonArray data;

    const auto labels = bookmarks->keys();
    for (const auto &label : labels) {
        const auto coordinates = bookmarks->value(label);
        data.append(QJsonObject { { s_bookmarksDataLabel, label },
                                  { s_bookmarksDataLon, coordinates.lon() },
                                  { s_bookmarksDataLat, coordinates.lat() },
                                  { s_bookmarksDataAlt, coordinates.alt() } });
    }

    setValue(s_bookmarks_version, s_bookmarksDataVersion);
    setValue(s_bookmarks_data, QJsonDocument(data).toJson(QJsonDocument::Compact));
}

QHash<QString, Coordinates> Settings::bookmarks() const
{
    const auto version = value(s_bookmarks_version, 0).toInt();
    if (version != s_bookmarksDataVersion) {
        return {};
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(
        value(s_bookmarks_data, QByteArray()).toByteArray(), &error);
    if (error.error != QJsonParseError::NoError || ! document.isArray()) {
        return {};
    }

    QHash<QString, Coordinates> bookmarks;

    const auto data = document.array();
    for (const auto &entry : data) {
        auto entryData = entry.toObject();
        const auto label = entryData.value(s_bookmarksDataLabel);
        const auto lon = entryData.value(s_bookmarksDataLon);
        const auto lat = entryData.value(s_bookmarksDataLat);
        const auto alt = entryData.value(s_bookmarksDataAlt);

        if (! label.isString() || ! lon.isDouble() || ! lat.isDouble() || ! alt.isDouble()) {
            return {};
        }

        bookmarks.insert(label.toString(),
                         Coordinates(lon.toDouble(), lat.toDouble(), alt.toDouble(), true));
    }

    return bookmarks;
}

void Settings::saveSplitImagesList(bool state)
{
    setValue(s_main_spitImagesList, state);
}

bool Settings::splitImagesList() const
{
    return value(s_main_spitImagesList, true).toBool();
}
