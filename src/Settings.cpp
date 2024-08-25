// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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

// Coordinates
static const QLatin1String s_coordinates("coordinates");
static const QLatin1String s_latBeforeLon("latBeforeLon");
static const QLatin1String s_coordinatesFlavor("flavor");
static const QHash<KGeoTag::CoordinatesFlavor, QString> s_flavorEnumToString {
    { KGeoTag::DecimalDegrees,               QStringLiteral("DecimalDegrees") },
    { KGeoTag::DegreesDecimalMinutes,        QStringLiteral("DegreesDecimalMinutes") },
    { KGeoTag::DegreesMinutesDecimalSeconds, QStringLiteral("DegreesMinutesDecimalSeconds") }
};
static const QHash<QString, KGeoTag::CoordinatesFlavor> s_flavorStringToEnum {
    { QStringLiteral("DecimalDegrees"),               KGeoTag::DecimalDegrees },
    { QStringLiteral("DegreesDecimalMinutes"),        KGeoTag::DegreesDecimalMinutes },
    { QStringLiteral("DegreesMinutesDecimalSeconds"), KGeoTag::DegreesMinutesDecimalSeconds }
};

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

static const QLatin1String s_assignment("assignment");

static const QLatin1String s_exactMatchTolerance("exactMatchTolerance");
static const QLatin1String s_maximumInterpolationInterval("maximumInterpolationInterval");
static const QLatin1String s_maximumInterpolationDistance("maximumInterpolationDistance");
static const QLatin1String s_excludeManuallyTaggedWhenReassigning(
                               "excludeManuallyTaggedWhenReassigning");

static const QLatin1String s_defaultMatchingMode("defaultMatchingMode");
static const QVector<KGeoTag::SearchType> s_defaultMatchingModeEnum {
    KGeoTag::CombinedMatchSearch,
    KGeoTag::ExactMatchSearch,
    KGeoTag::InterpolatedMatchSearch
};
static const QVector<QString> s_defaultMatchingModeString {
    QStringLiteral("combinedSearch"),
    QStringLiteral("exactSearch"),
    QStringLiteral("interpolatedSearch")
};
static const QString &s_defaultMatchingModePreset = s_defaultMatchingModeString.at(0);

static const QLatin1String s_selectNextUntagged("selectNextUntagged");

// Elevation lookup
static const QLatin1String s_elevationLookup("elevationLookup");
static const QLatin1String s_lookupElevationAutomatically("lookupElevationAutomatically");
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
static const QLatin1String s_dataset("dataset");

// Saving
static const QLatin1String s_saving("saving");
static const QVector<QString> s_writeModes = {
    QStringLiteral("WRITETOIMAGEONLY"),
    QStringLiteral("WRITETOSIDECARONLY"),
    QStringLiteral("WRITETOSIDECARANDIMAGE")
};
static const QString &s_defaultWriteMode = s_writeModes.at(0);
static const QLatin1String s_writeMode("writeMode");
static const QLatin1String s_allowWriteRawFiles("allowWriteRawFiles");
static const QLatin1String s_createBackups("createBackups");

// Bookmarks

static const QLatin1String s_bookmarks("bookmarks");
static const QLatin1String s_version("version");
static const QLatin1String s_data("data");

static constexpr int s_bookmarksDataVersion = 1;
static const QString s_bookmarksDataLabel = QStringLiteral("label");
static const QString s_bookmarksDataLon = QStringLiteral("lon");
static const QString s_bookmarksDataLat = QStringLiteral("lat");
static const QString s_bookmarksDataAlt = QStringLiteral("alt");

Settings::Settings(QObject *parent) : QObject(parent)
{
    m_config = KSharedConfig::openConfig();

    // Initialize the "lat before lon" bool
    auto group = m_config->group(s_coordinates);
    m_latBeforeLon = group.readEntry(s_latBeforeLon, false);

    // Initialize the coordinates flavor
    const auto flavor = group.readEntry(s_coordinatesFlavor,
                                        s_flavorEnumToString.value(KGeoTag::DecimalDegrees));
    m_coordinatesFlavor = s_flavorStringToEnum.contains(flavor)
                              ? s_flavorStringToEnum.value(flavor)
                              : KGeoTag::DecimalDegrees;
}

// KMainWindow settings

QByteArray Settings::mainWindowState() const
{
    auto group = m_config->group(QStringLiteral("MainWindow"));
    const auto state = group.readEntry(QStringLiteral("State"), QString());
    return QByteArray::fromBase64(state.toUtf8());
}

// Main

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

// Assignment

void Settings::saveExactMatchTolerance(int seconds)
{
    auto group = m_config->group(s_assignment);
    group.writeEntry(s_exactMatchTolerance, seconds);
    group.sync();
}

int Settings::exactMatchTolerance() const
{
    auto group = m_config->group(s_assignment);
    return group.readEntry(s_exactMatchTolerance, 10);
}

void Settings::saveMaximumInterpolationInterval(int seconds)
{
    auto group = m_config->group(s_assignment);
    group.writeEntry(s_maximumInterpolationInterval, seconds);
    group.sync();
}

int Settings::maximumInterpolationInterval() const
{
    auto group = m_config->group(s_assignment);
    return group.readEntry(s_maximumInterpolationInterval, -1);
}

void Settings::saveMaximumInterpolationDistance(int meters)
{
    auto group = m_config->group(s_assignment);
    group.writeEntry(s_maximumInterpolationDistance, meters);
    group.sync();
}

int Settings::maximumInterpolationDistance() const
{
    auto group = m_config->group(s_assignment);
    return group.readEntry(s_maximumInterpolationDistance, -1);
}

void Settings::saveExcludeManuallyTaggedWhenReassigning(bool state)
{
    auto group = m_config->group(s_assignment);
    group.writeEntry(s_excludeManuallyTaggedWhenReassigning, state);
    group.sync();
}

bool Settings::excludeManuallyTaggedWhenReassigning() const
{
    auto group = m_config->group(s_assignment);
    return group.readEntry(s_excludeManuallyTaggedWhenReassigning, true);
}

// Elevation lookup

void Settings::saveLookupElevationAutomatically(bool state)
{
    auto group = m_config->group(s_elevationLookup);
    group.writeEntry(s_lookupElevationAutomatically, state);
    group.sync();
}

bool Settings::lookupElevationAutomatically() const
{
    auto group = m_config->group(s_elevationLookup);
    return group.readEntry(s_lookupElevationAutomatically, false);
}

void Settings::saveElevationDataset(const QString &id)
{
    auto group = m_config->group(s_elevationLookup);
    group.writeEntry(s_dataset, id);
    group.sync();
}

QString Settings::elevationDataset() const
{
    auto group = m_config->group(s_elevationLookup);
    const auto dataset = group.readEntry(s_dataset, s_defaultElevationDataset);
    return s_elevationDatasets.contains(dataset) ? dataset : s_defaultElevationDataset;
}

// Saving

void Settings::saveWriteMode(const QString &writeMode)
{
    auto group = m_config->group(s_saving);
    group.writeEntry(s_writeMode, writeMode);
    group.sync();
}

QString Settings::writeMode() const
{
    auto group = m_config->group(s_saving);
    const auto mode = group.readEntry(s_writeMode, s_defaultWriteMode);
    return s_writeModes.contains(mode) ? mode : s_defaultWriteMode;
}

void Settings::saveCreateBackups(bool state)
{
    auto group = m_config->group(s_saving);
    group.writeEntry(s_createBackups, state);
    group.sync();
}

bool Settings::createBackups() const
{
    auto group = m_config->group(s_saving);
    return group.readEntry(s_createBackups, true);
}

void Settings::saveAllowWriteRawFiles(bool state)
{
    auto group = m_config->group(s_saving);
    group.writeEntry(s_allowWriteRawFiles, state);
    group.sync();
}

bool Settings::allowWriteRawFiles() const
{
    auto group = m_config->group(s_saving);
    return group.readEntry(s_allowWriteRawFiles, false);
}

// Bookmarks

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

    auto group = m_config->group(s_bookmarks);
    group.writeEntry(s_version, s_bookmarksDataVersion);
    group.writeEntry(s_data, QJsonDocument(data).toJson(QJsonDocument::Compact));
    group.sync();
}

QHash<QString, Coordinates> Settings::bookmarks() const
{
    auto group = m_config->group(s_bookmarks);

    const auto version = group.readEntry(s_version, 0);
    if (version != s_bookmarksDataVersion) {
        return {};
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(group.readEntry(s_data, QByteArray()), &error);
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

void Settings::saveDefaultMatchingMode(KGeoTag::SearchType mode)
{
    auto group = m_config->group(s_assignment);
    group.writeEntry(s_defaultMatchingMode,
        s_defaultMatchingModeString.at(s_defaultMatchingModeEnum.indexOf(mode)));
    group.sync();
}

KGeoTag::SearchType Settings::defaultMatchingMode() const
{
    auto group = m_config->group(s_assignment);
    const auto modeString = group.readEntry(s_defaultMatchingMode, s_defaultMatchingModePreset);
    if (s_defaultMatchingModeString.contains(modeString)) {
        return s_defaultMatchingModeEnum.at(s_defaultMatchingModeString.indexOf(modeString));
    } else {
        return KGeoTag::CombinedMatchSearch;
    }
}

void Settings::saveSelectNextUntagged(bool state)
{
    auto group = m_config->group(s_assignment);
    group.writeEntry(s_selectNextUntagged, state);
    group.sync();
}

bool Settings::selectNextUntagged() const
{
    auto group = m_config->group(s_assignment);
    return group.readEntry(s_selectNextUntagged, true);
}

void Settings::saveLatBeforeLon(bool state)
{
    m_latBeforeLon = state;
    auto group = m_config->group(s_coordinates);
    group.writeEntry(s_latBeforeLon, state);
    group.sync();
}

bool Settings::latBeforeLon() const
{
    return m_latBeforeLon;
}

void Settings::saveCoordinatesFlavor(KGeoTag::CoordinatesFlavor flavor)
{
    m_coordinatesFlavor = flavor;
    auto group = m_config->group(s_coordinates);
    group.writeEntry(s_coordinatesFlavor, s_flavorEnumToString.value(flavor));
    group.sync();
}

KGeoTag::CoordinatesFlavor Settings::coordinatesFlavor() const
{
    return m_coordinatesFlavor;
}
