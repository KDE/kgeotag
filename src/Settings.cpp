/* SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "Settings.h"

// KDE includes
#include <KConfigGroup>

// Qt includes
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

// Main
static const QLatin1String s_main("main");
static const QLatin1String s_windowState("windowState");
static const QLatin1String s_lastOpenPath("lastOpenPath");
static const QLatin1String s_splitImagesList("splitImagesList");

// Map
static const QLatin1String s_map("map");
static const QLatin1String s_showCrosshairs("showCrosshairs");
static const QLatin1String s_centerLon("centerLon");
static const QLatin1String s_centerLat("centerLat");
static const QLatin1String s_zoom("zoom");

// Floaters visibility
static const QLatin1String s_floatersVisibility("floatersVisibility");

// Tracks
static const QLatin1String s_track("tracks");
static const QLatin1String s_color("color");
static const QLatin1String s_width("width");
static const QLatin1String s_style("style");
static const QVector<Qt::PenStyle> s_trackStyleEnum {
    Qt::SolidLine,
    Qt::DashLine,
    Qt::DotLine,
    Qt::DashDotLine,
    Qt::DashDotDotLine
};
static const QVector<QString> s_trackStyleString {
    QStringLiteral("solid"),
    QStringLiteral("dashes"),
    QStringLiteral("dots"),
    QStringLiteral("dash_dot"),
    QStringLiteral("dash_dot_dot")
};

// Images
static const QLatin1String s_images("images");
static const QLatin1String s_thumnailSize("thumbnailSize");
static const QLatin1String s_previewSize("previewSize");

// Assignment

static const QLatin1String s_assignment("assignment/");
static const QString s_assignment_exactMatchTolerance
    = s_assignment + QLatin1String("exact_match_tolerance");
static const QString s_assignment_maximumInterpolationInterval
    = s_assignment + QLatin1String("maximum_interpolation_interval");
static const QString s_assignment_maximumInterpolationDistance
    = s_assignment + QLatin1String("maximum_interpolation_distance");
static const QString s_assignment_excludeManuallyTaggedWhenReassigning
    = s_assignment + QLatin1String("exclude_manually_tagged_when_reassigning");

// Elevation lookup

static const QLatin1String s_elevationLookup("elevation_lookup/");

static const QString s_elevationLookup_automaticLookup
    = s_elevationLookup + QLatin1String("automatic_lookup");

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
    m_config = KSharedConfig::openConfig();
}

// Main

void Settings::saveMainWindowState(const QByteArray &data)
{
    auto group = m_config->group(s_main);
    group.writeEntry(s_windowState, data);
    group.sync();
}

QByteArray Settings::mainWindowState() const
{
    auto group = m_config->group(s_main);
    return group.readEntry(s_windowState, QByteArray());
}

void Settings::saveLastOpenPath(const QString &path)
{
    auto group = m_config->group(s_main);
    group.writeEntry(s_lastOpenPath, path);
    group.sync();
}

QString Settings::lastOpenPath() const
{
    auto group = m_config->group(s_main);
    return group.readEntry(s_lastOpenPath, QString());
}

void Settings::saveSplitImagesList(bool state)
{
    auto group = m_config->group(s_main);
    group.writeEntry(s_splitImagesList, state);
    group.sync();
}

bool Settings::splitImagesList() const
{
    auto group = m_config->group(s_main);
    return group.readEntry(s_splitImagesList, true);
}

// Map

void Settings::saveShowCrosshairs(bool state)
{
    auto group = m_config->group(s_map);
    group.writeEntry(s_showCrosshairs, state);
    group.sync();
}

bool Settings::showCrosshairs() const
{
    auto group = m_config->group(s_map);
    return group.readEntry(s_showCrosshairs, true);
}

void Settings::saveMapCenter(const Coordinates &coordinates)
{
    auto group = m_config->group(s_map);
    group.writeEntry(s_centerLon, coordinates.lon());
    group.writeEntry(s_centerLat, coordinates.lat());
    group.sync();
}

Coordinates Settings::mapCenter() const
{
    auto group = m_config->group(s_map);
    return Coordinates(group.readEntry(s_centerLon, 0.0),
                       group.readEntry(s_centerLat, 0.0),
                       0.0,
                       true);
}

void Settings::saveZoom(int zoom)
{
    auto group = m_config->group(s_map);
    group.writeEntry(s_zoom, zoom);
    group.sync();
}

int Settings::zoom() const
{
    auto group = m_config->group(s_map);
    return group.readEntry(s_zoom, 1520);
}

// Floaters visibility

void Settings::saveFloatersVisibility(const QHash<QString, bool> &data)
{
    auto group = m_config->group(s_floatersVisibility);
    const auto keys = data.keys();
    for (const auto &key : keys) {
        group.writeEntry(key, data.value(key));
    }
    group.sync();
}

QHash<QString, bool> Settings::floatersVisibility()
{
    QHash<QString, bool> data;
    auto group = m_config->group(s_floatersVisibility);

    const auto keys = group.keyList();
    for (const auto &key : keys) {
        data.insert(key, group.readEntry(key, true));
    }

    return data;
}

// Tracks

void Settings::saveTrackColor(const QColor &color)
{
    auto group = m_config->group(s_track);
    group.writeEntry(s_color, color);
    group.sync();
}

QColor Settings::trackColor() const
{
    auto group = m_config->group(s_track);
    return group.readEntry(s_color, QColor(255, 0, 255, 150));
}

void Settings::saveTrackStyle(Qt::PenStyle style)
{
    auto group = m_config->group(s_track);
    group.writeEntry(s_style, s_trackStyleString.at(s_trackStyleEnum.indexOf(style)));
    group.sync();
}

Qt::PenStyle Settings::trackStyle() const
{
    auto group = m_config->group(s_track);
    QString styleString = group.readEntry(s_style, QStringLiteral("dots"));
    if (s_trackStyleString.contains(styleString)) {
        return s_trackStyleEnum.at(s_trackStyleString.indexOf(styleString));
    } else {
        return Qt::DotLine;
    }
}

void Settings::saveTrackWidth(int width)
{
    auto group = m_config->group(s_track);
    group.writeEntry(s_width, width);
    group.sync();
}

int Settings::trackWidth() const
{
    auto group = m_config->group(s_track);
    return group.readEntry(s_width, 3);
}

// Images

void Settings::saveThumbnailSize(int size)
{
    auto group = m_config->group(s_images);
    group.writeEntry(s_thumnailSize, size);
    group.sync();
}

int Settings::thumbnailSize() const
{
    auto group = m_config->group(s_images);
    return group.readEntry(s_thumnailSize, 32);
}

void Settings::savePreviewSize(int size)
{
    auto group = m_config->group(s_images);
    group.writeEntry(s_previewSize, size);
    group.sync();
}

int Settings::previewSize() const
{
    auto group = m_config->group(s_images);
    return group.readEntry(s_previewSize, 400);
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

void Settings::saveExcludeManuallyTaggedWhenReassigning(bool state)
{
    setValue(s_assignment_excludeManuallyTaggedWhenReassigning, state);
}

bool Settings::excludeManuallyTaggedWhenReassigning() const
{
    return value(s_assignment_excludeManuallyTaggedWhenReassigning, true).toBool();
}

void Settings::saveLookupElevationAutomatically(bool state)
{
    setValue(s_elevationLookup_automaticLookup, state);
}

bool Settings::lookupElevationAutomatically() const
{
    return value(s_elevationLookup_automaticLookup, false).toBool();
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
