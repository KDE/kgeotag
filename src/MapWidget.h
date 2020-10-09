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

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

// Marble includes
#include <marble/MarbleWidget.h>
#include <marble/GeoDataCoordinates.h>

// Qt includes
#include <QHash>

// Local classes
class Settings;
class ImageCache;

// Qt classes
class QDragEnterEvent;
class QDropEvent;

class MapWidget : public Marble::MarbleWidget
{
    Q_OBJECT

public:
    explicit MapWidget(Settings *settings, ImageCache *imageCache, QWidget *parent = nullptr);
    virtual void customPaint(Marble::GeoPainter *painter) override;
    void saveSettings();
    void restoreSettings();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private: // Variables
    Settings *m_settings;
    ImageCache *m_imageCache;
    QHash<QString, Marble::GeoDataCoordinates> m_images;

};

#endif // MAPWIDGET_H
