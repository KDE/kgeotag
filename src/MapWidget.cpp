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
#include "Coordinates.h"

// Marble includes
#include <marble/MarbleWidget.h>
#include <marble/AbstractFloatItem.h>

// Qt includes
#include <QVBoxLayout>
#include <QDebug>

MapWidget::MapWidget(Settings *settings, QWidget *parent)
    : QWidget(parent), m_settings(settings)
{
    auto *layout = new QVBoxLayout(this);

    m_mapWidget = new Marble::MarbleWidget;
    m_mapWidget->setProjection(Marble::Mercator);
    m_mapWidget->setMapThemeId(QStringLiteral("earth/openstreetmap/openstreetmap.dgml"));

    layout->addWidget(m_mapWidget);
    m_mapWidget->show();
}

void MapWidget::saveSettings()
{
    // Save the floaters visibility

    QHash<QString, bool> visibility;

    const auto floatItems = m_mapWidget->floatItems();
    for (const auto &item : floatItems) {
        visibility.insert(item->name(), item->visible());
    }

    m_settings->saveFloatersVisibility(visibility);

    // Save the current center point
    const auto center = m_mapWidget->focusPoint();
    m_settings->saveMapCenter(Coordinates::Data {
                                  center.longitude(Marble::GeoDataCoordinates::Degree),
                                  center.latitude(Marble::GeoDataCoordinates::Degree) });

    // Save the zoom level
    m_settings->saveZoom(m_mapWidget->zoom());
}

void MapWidget::restoreSettings()
{
    // Restore the floaters visiblility
    const auto floatersVisiblility = m_settings->floatersVisibility();
    const auto floatItems = m_mapWidget->floatItems();
    for (const auto &item : floatItems) {
        const auto name = item->name();
        if (floatersVisiblility.contains(name)) {
            item->setVisible(floatersVisiblility.value(name));
        }
    }

    // Restore map's last center point
    const auto [ lon, lat ] = m_settings->mapCenter();
    m_mapWidget->centerOn(lon, lat);

    // Restore the last zoom level
    m_mapWidget->setZoom(m_settings->zoom());
}
