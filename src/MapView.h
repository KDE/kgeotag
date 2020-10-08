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

#ifndef MAPVIEW_H
#define MAPVIEW_H

// Marble includes
#include <marble/MarbleWidget.h>
#include <marble/GeoDataCoordinates.h>

// Qt includes
#include <QHash>

// Local classes
class ImageCache;

// Qt classes
class QDragEnterEvent;
class QDropEvent;

class MapView : public Marble::MarbleWidget
{
    Q_OBJECT

public:
    explicit MapView(ImageCache *imageCache, QWidget *parent = nullptr);
    virtual void customPaint(Marble::GeoPainter *painter) override;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private: // Variables
    ImageCache *m_imageCache;
    QHash<QString, Marble::GeoDataCoordinates> m_images;

};

#endif // MAPVIEW_H
