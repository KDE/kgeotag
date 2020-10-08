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
#include "MapView.h"
#include "ImageCache.h"

// Marble includes
#include <marble/GeoPainter.h>

// Qt includes
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDropEvent>

MapView::MapView(ImageCache *imageCache, QWidget *parent)
    : Marble::MarbleWidget(parent), m_imageCache(imageCache)
{
    setAcceptDrops(true);

    setProjection(Marble::Mercator);
    setMapThemeId(QStringLiteral("earth/openstreetmap/openstreetmap.dgml"));
}

void MapView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(QStringLiteral("text/plain"))
        && m_imageCache->contains(event->mimeData()->text())) {

        event->acceptProposedAction();
    }
}

void MapView::dropEvent(QDropEvent *event)
{
    const auto dropPosition = event->pos();

    qreal lon;
    qreal lat;
    if (! geoCoordinates(dropPosition.x(), dropPosition.y(), lon, lat,
                         Marble::GeoDataCoordinates::Degree)) {
        return;
    }

    m_images[event->mimeData()->text()]
        = Marble::GeoDataCoordinates(lon, lat, 0.0, Marble::GeoDataCoordinates::Degree);
    reloadMap();

    event->acceptProposedAction();
}

void MapView::customPaint(Marble::GeoPainter *painter)
{
    const auto images = m_images.keys();
    for (const auto image : images) {
        painter->drawPixmap(m_images.value(image),
                            QPixmap::fromImage(m_imageCache->thumbnail(image)));
    }
}
