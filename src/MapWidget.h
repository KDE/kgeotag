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

// Local includes
#include "Coordinates.h"

// Marble includes
#include <marble/MarbleWidget.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLineString.h>

// Qt includes
#include <QHash>
#include <QPen>
#include <QDateTime>

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
    void addGpx(const QString &path);
    void addImage(const QString &path, double lon, double lat);
    void addImage(const QString &path, const Coordinates::Data &coordinates);
    Coordinates::Data findExactCoordinates(const QDateTime &time) const;
    Coordinates::Data findInterpolatedCoordinates(const QDateTime &time) const;

signals:
    void imageAssigned(const QString &path) const;

public slots:
    void centerImage(const QString &path);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private: // Variables
    Settings *m_settings;
    ImageCache *m_imageCache;
    QHash<QString, Marble::GeoDataCoordinates> m_images;
    QVector<Marble::GeoDataLineString> m_tracks;
    QPen m_trackPen;
    QHash<QDateTime, Coordinates::Data> m_points;
    QDateTime m_first;
    QDateTime m_last;

};

#endif // MAPWIDGET_H
