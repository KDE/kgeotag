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
#include "SharedObjects.h"
#include "Settings.h"
#include "ImageCache.h"
#include "GpxEngine.h"
#include "ElevationEngine.h"
#include "MapWidget.h"
#include "CoordinatesFormatter.h"

SharedObjects::SharedObjects(QObject *parent) : QObject(parent)
{
    m_settings = new Settings(this);
    m_imageCache = new ImageCache(this, m_settings);
    m_gpxEngine = new GpxEngine(this, m_settings);
    m_elevationEngine = new ElevationEngine(this, m_settings);
    m_mapWidget = new MapWidget(this);
    m_coordinatesFormatter = new CoordinatesFormatter(this, &m_locale);
}

Settings *SharedObjects::settings() const
{
    return m_settings;
}

ImageCache *SharedObjects::imageCache() const
{
    return m_imageCache;
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
