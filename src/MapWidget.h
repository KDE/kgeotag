// SPDX-FileCopyrightText: 2020-2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// Marble includes
#include <marble/MarbleWidget.h>
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLineString.h>

// Qt includes
#include <QHash>
#include <QPen>
#include <QDateTime>
#include <QMenu>

// Local classes
class SharedObjects;
class Settings;
class GeoDataModel;
class ImagesModel;
class CoordinatesFormatter;

// Qt classes
class QDragEnterEvent;
class QDropEvent;

class MapWidget : public Marble::MarbleWidget
{
    Q_OBJECT

public:
    explicit MapWidget(SharedObjects *sharedObjects, QWidget *parent = nullptr);
    void updateSettings();
    void saveSettings();
    void restoreSettings();
    void zoomToTrack(const QModelIndex &index);
    void zoomToTracks(const QList<QString> &paths);
    void centerImage(const QModelIndex &index);
    void centerCoordinates(const Coordinates &coordinates);
    Coordinates currentCenter() const;
    QMenu *mapCenterMenu() const;

Q_SIGNALS:
    void mapMoved(const Coordinates &center);
    void imagesDropped(const QList<QString> &paths);
    void requestLoadGpx(const QList<QString> &paths);
    void requestAddBookmark();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private Q_SLOTS:
    void showContextMenu(int x, int y);
    void changeFloaterVisiblity(QAction *action);

private: // Variables
    Settings *m_settings;
    GeoDataModel *m_geoDataModel;
    ImagesModel *m_imagesModel;
    CoordinatesFormatter *m_coordinatesFormatter;
    QList<Marble::GeoDataLineString> m_tracks;
    QPen m_trackPen;
    QMenu *m_contextMenu;
    QMenu *m_mapCenterMenu;
    QList<QAction *> m_floatersActions;

};

#endif // MAPWIDGET_H
