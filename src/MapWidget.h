/* SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

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
    void zoomToTrack(const QString &path);
    void zoomToTracks(const QVector<QString> &paths);
    void centerImage(const QModelIndex &index);
    void centerCoordinates(const Coordinates &coordinates);
    Coordinates currentCenter() const;

signals:
    void mapMoved(const Coordinates &center);
    void imagesDropped(const QVector<QString> &paths);
    void requestLoadGpx(const QVector<QString> &paths);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void showContextMenu(int x, int y);
    void changeFloaterVisiblity(QAction *action);

private: // Variables
    Settings *m_settings;
    GeoDataModel *m_geoDataModel;
    ImagesModel *m_imagesModel;
    QVector<Marble::GeoDataLineString> m_tracks;
    QPen m_trackPen;
    QMenu *m_contextMenu;
    QVector<QAction *> m_floatersActions;

};

#endif // MAPWIDGET_H
