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

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

// Local includes
#include "KGeoTag.h"

// Marble includes
#include <marble/MarbleWidget.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLineString.h>
#include <marble/GeoDataLatLonAltBox.h>

// Qt includes
#include <QHash>
#include <QPen>
#include <QDateTime>
#include <QMenu>

// Local classes
class SharedObjects;
class Settings;
class ImageCache;
class ImagesModel;

// Qt classes
class QDragEnterEvent;
class QDropEvent;

class MapWidget : public Marble::MarbleWidget
{
    Q_OBJECT

public:
    explicit MapWidget(SharedObjects *sharedObjects, QWidget *parent = nullptr);
    void updateSettings();
    virtual void customPaint(Marble::GeoPainter *painter) override;
    void saveSettings();
    void restoreSettings();
    void addImage(const QString &path, double lon, double lat);
    void addImage(const QString &path, const KGeoTag::Coordinates &coordinates);
    void removeImage(const QString &path);
    void centerImage(const QString &path);
    void centerCoordinates(const KGeoTag::Coordinates &coordinates);
    void zoomToGpxBox();
    KGeoTag::Coordinates currentCenter() const;

public slots:
    void addSegment(const QVector<KGeoTag::Coordinates> &segment);

signals:
    void imagesDropped(const QVector<QString> &paths) const;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private slots:
    void showContextMenu(int x, int y);
    void changeFloaterVisiblity(QAction *action);

private: // Variables
    Settings *m_settings;
    ImageCache *m_imageCache;
    ImagesModel *m_imagesModel;
    QHash<QString, Marble::GeoDataCoordinates> m_images;
    QVector<Marble::GeoDataLineString> m_tracks;
    QPen m_trackPen;
    Marble::GeoDataLatLonAltBox m_gpxBox;
    QMenu *m_contextMenu;
    QVector<QAction *> m_floatersActions;

};

#endif // MAPWIDGET_H
