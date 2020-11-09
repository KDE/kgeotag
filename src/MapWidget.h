/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>
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
#include <marble/GeoDataLatLonAltBox.h>

// Qt includes
#include <QHash>
#include <QPen>
#include <QDateTime>
#include <QMenu>

// Local classes
class SharedObjects;
class Settings;
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
    void centerImage(const QModelIndex &index);
    void centerCoordinates(const Coordinates &coordinates);
    void zoomToGpxBox();
    Coordinates currentCenter() const;

public slots:
    void addSegment(const QVector<Coordinates> &segment);

signals:
    void imagesDropped(const QVector<QString> &paths) const;
    void requestLoadGpx(const QVector<QString> &paths) const;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private slots:
    void showContextMenu(int x, int y);
    void changeFloaterVisiblity(QAction *action);

private: // Variables
    Settings *m_settings;
    ImagesModel *m_imagesModel;
    QVector<Marble::GeoDataLineString> m_tracks;
    QPen m_trackPen;
    Marble::GeoDataLatLonAltBox m_gpxBox;
    QMenu *m_contextMenu;
    QVector<QAction *> m_floatersActions;

};

#endif // MAPWIDGET_H
