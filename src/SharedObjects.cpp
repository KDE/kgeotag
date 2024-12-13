// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "SharedObjects.h"
#include "Settings.h"
#include "ImagesModel.h"
#include "GeoDataModel.h"
#include "GpxEngine.h"
#include "ElevationEngine.h"
#include "MapWidget.h"
#include "CoordinatesFormatter.h"
#include "CoordinatesParser.h"

SharedObjects::SharedObjects(QObject *parent) : QObject(parent)
{
    m_settings = new Settings(this);
    m_imagesModel = new ImagesModel(this, m_settings->splitImagesList(),
                                    m_settings->thumbnailSize(), m_settings->previewSize());
    m_geoDataModel = new GeoDataModel(this);
    m_gpxEngine = new GpxEngine(this, m_geoDataModel);
    m_elevationEngine = new ElevationEngine(this, m_settings);
    m_coordinatesFormatter = new CoordinatesFormatter(this, &m_locale, m_settings);
    m_coordinatesParser = new CoordinatesParser(this, &m_locale);
    m_mapWidget = new MapWidget(this);
}

Settings *SharedObjects::settings() const
{
    return m_settings;
}

ImagesModel *SharedObjects::imagesModel() const
{
    return m_imagesModel;
}

GeoDataModel *SharedObjects::geoDataModel() const
{
    return m_geoDataModel;
}

GpxEngine *SharedObjects::gpxEngine() const
{
    return m_gpxEngine;
}

ElevationEngine *SharedObjects::elevationEngine() const
{
    return m_elevationEngine;
}

MapWidget *SharedObjects::mapWidget() const
{
    return m_mapWidget;
}

CoordinatesFormatter *SharedObjects::coordinatesFormatter() const
{
    return m_coordinatesFormatter;
}

CoordinatesParser *SharedObjects::coordinatesParser() const
{
    return m_coordinatesParser;
}

void SharedObjects::setBookmarks(const QHash<QString, Coordinates> *bookmarks)
{
    m_bookmarks = bookmarks;
}

const QHash<QString, Coordinates> *SharedObjects::bookmarks() const
{
    return m_bookmarks;
}

const QLocale *SharedObjects::locale() const
{
    return &m_locale;
}
