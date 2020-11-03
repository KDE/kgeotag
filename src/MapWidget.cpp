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
#include "MapWidget.h"
#include "SharedObjects.h"
#include "Settings.h"
#include "KGeoTag.h"
#include "ImagesModel.h"

// Marble includes
#include <marble/GeoPainter.h>
#include <marble/AbstractFloatItem.h>
#include <marble/MarbleWidgetInputHandler.h>

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QMenu>
#include <QAction>
#include <QUrl>

// C++ includes
#include <functional>
#include <utility>

static QString s_licenseFloaterId = QStringLiteral("license");
static QVector<QString> s_unsupportedFloaters = {
    s_licenseFloaterId,
    QStringLiteral("elevationprofile"),
    QStringLiteral("GpsInfo"),
    QStringLiteral("progress"),
    QStringLiteral("routing"),
    QStringLiteral("speedometer")
};

MapWidget::MapWidget(SharedObjects *sharedObjects, QWidget *parent)
    : Marble::MarbleWidget(parent),
      m_settings(sharedObjects->settings()),
      m_imagesModel(sharedObjects->imagesModel())
{
    setAcceptDrops(true);

    setProjection(Marble::Mercator);
    setMapThemeId(QStringLiteral("earth/openstreetmap/openstreetmap.dgml"));

    m_trackPen.setCapStyle(Qt::RoundCap);
    m_trackPen.setJoinStyle(Qt::RoundJoin);
    updateSettings();

    // Build a context menu

    m_contextMenu = new QMenu(this);
    m_contextMenu->addSection(i18n("Floating items"));

    const auto floatItemsList = floatItems();
    const auto visibility = m_settings->floatersVisibility();

    // Add a "Toggle crosshairs" action
    auto *crossHairsAction = m_contextMenu->addAction(i18n("Crosshairs"));
    crossHairsAction->setCheckable(true);
    connect(crossHairsAction, &QAction::toggled, this, &Marble::MarbleWidget::setShowCrosshairs);
    crossHairsAction->setChecked(m_settings->showCrosshairs());
    setShowCrosshairs(m_settings->showCrosshairs());

    // Add actions for all supported floaters

    m_contextMenu->addSeparator();

    for (const auto &item : floatItemsList) {
        const auto id = item->nameId();
        if (s_unsupportedFloaters.contains(id)) {
            auto item = floatItem(id);
            if (item != nullptr) {
                item->setVisible(false);
            }
            continue;
        }

        auto *action = m_contextMenu->addAction(item->name());
        action->setIcon(item->icon());
        action->setData(id);
        action->setCheckable(true);
        action->setChecked(visibility.value(id));
        action->setData(id);

        m_floatersActions.append(action);

        connect(action, &QAction::toggled,
                this, std::bind(&MapWidget::changeFloaterVisiblity, this, action));
    }

    // The "License" floater is always shown on startup (by Marble itself)
    auto *licenseFloater = floatItem(s_licenseFloaterId);
    if (licenseFloater != nullptr) {
        m_contextMenu->addSeparator();
        auto *licenseAction = m_contextMenu->addAction(licenseFloater->name());
        licenseAction->setCheckable(true);
        licenseAction->setChecked(true);
        licenseAction->setData(s_licenseFloaterId);
        connect(licenseAction, &QAction::toggled,
                this, std::bind(&MapWidget::changeFloaterVisiblity, this, licenseAction));
        m_floatersActions.append(licenseAction);
    }

    // Don't use the MarbleWidget context menu, but our own
    auto *handler = inputHandler();
    handler->setMouseButtonPopupEnabled(Qt::RightButton, false);
    connect(handler, &Marble::MarbleWidgetInputHandler::rmbRequest,
            this, &MapWidget::showContextMenu);
}

void MapWidget::showContextMenu(int x, int y)
{
    for (auto *action : std::as_const(m_floatersActions)) {
        action->blockSignals(true);
        action->setChecked(floatItem(action->data().toString())->visible());
        action->blockSignals(false);
    }

    m_contextMenu->exec(mapToGlobal(QPoint(x, y)));
}

void MapWidget::changeFloaterVisiblity(QAction *action)
{
    floatItem(action->data().toString())->setVisible(action->isChecked());
}

void MapWidget::customPaint(Marble::GeoPainter *painter)
{
    const auto images = m_images.keys();
    for (const auto &image : images) {
        painter->drawPixmap(m_images.value(image),
                            QPixmap::fromImage(m_imagesModel->thumbnail(image)));
    }

    painter->setPen(m_trackPen);
    for (const auto &lineString : m_tracks) {
        painter->drawPolyline(lineString);
    }
}

void MapWidget::updateSettings()
{
    m_trackPen.setColor(m_settings->trackColor());
    m_trackPen.setWidth(m_settings->trackWidth());
    m_trackPen.setStyle(m_settings->trackStyle());
    reloadMap();
}

void MapWidget::saveSettings()
{
    // Save the crosshairs visibility
    m_settings->saveShowCrosshairs(showCrosshairs());

    // Save the floaters visibility

    QHash<QString, bool> visibility;

    const auto floatItemsList = floatItems();
    for (const auto &item : floatItemsList) {
        const auto id = item->nameId();
        if (s_unsupportedFloaters.contains(id)) {
            continue;
        }
        visibility.insert(id, item->visible());
    }

    m_settings->saveFloatersVisibility(visibility);

    // Save the current center point
    m_settings->saveMapCenter(currentCenter());

    // Save the zoom level
    m_settings->saveZoom(zoom());
}

void MapWidget::restoreSettings()
{
    // Restore the floaters visiblility
    const auto floatersVisiblility = m_settings->floatersVisibility();
    const auto floatItemsList = floatItems();
    for (const auto &item : floatItemsList) {
        const auto name = item->nameId();
        if (floatersVisiblility.contains(name)) {
            item->setVisible(floatersVisiblility.value(name));
        }
    }

    // Restore map's last center point
    const auto [ lon, lat, alt, isSet ] = m_settings->mapCenter();
    centerOn(lon, lat);

    // Restore the last zoom level
    setZoom(m_settings->zoom());
}

void MapWidget::addSegment(const QVector<KGeoTag::Coordinates> &segment)
{
    Marble::GeoDataLineString lineString;

    for (const auto &coordinates : segment) {
        const Marble::GeoDataCoordinates marbleCoordinates
            = Marble::GeoDataCoordinates(coordinates.lon, coordinates.lat, 0.0,
                                         Marble::GeoDataCoordinates::Degree);
        lineString.append(marbleCoordinates);
    }

    m_tracks.append(lineString);

    const auto box = lineString.latLonAltBox();
    if (m_gpxBox.isEmpty()) {
        m_gpxBox = box;
    } else {
        m_gpxBox |= box;
    }
}

void MapWidget::dragEnterEvent(QDragEnterEvent *event)
{
    const auto mimeData = event->mimeData();

    if (! mimeData->hasFormat(QStringLiteral("text/uri-list"))) {
        return;
    }

    const auto urls = mimeData->urls();
    for (const auto &url : urls) {
        if (! m_imagesModel->contains(url.toLocalFile())) {
            return;
        }
    }

    event->acceptProposedAction();
}

void MapWidget::dropEvent(QDropEvent *event)
{
    const auto dropPosition = event->pos();

    qreal lon;
    qreal lat;
    if (! geoCoordinates(dropPosition.x(), dropPosition.y(), lon, lat,
                         Marble::GeoDataCoordinates::Degree)) {
        return;
    }

    QVector<QString> paths;
    const auto urls = event->mimeData()->urls();
    for (const auto &url : urls) {
        const auto path = url.toLocalFile();
        addImage(path, lon, lat);
        m_imagesModel->setCoordinates(path, lon, lat, 0.0);
        paths.append(path);
    }

    reloadMap();
    emit imagesDropped(paths);

    event->acceptProposedAction();
}

void MapWidget::addImage(const QString &path, const KGeoTag::Coordinates &coordinates)
{
    addImage(path, coordinates.lon, coordinates.lat);
}

void MapWidget::addImage(const QString &path, double lon, double lat)
{
    m_images[path] = Marble::GeoDataCoordinates(lon, lat, 0.0, Marble::GeoDataCoordinates::Degree);
}

void MapWidget::removeImage(const QString &path)
{
    m_images.remove(path);
}

void MapWidget::centerImage(const QString &path)
{
    centerCoordinates(m_imagesModel->coordinates(path));
}

void MapWidget::centerCoordinates(const KGeoTag::Coordinates &coordinates)
{
    centerOn(coordinates.lon, coordinates.lat);
}

void MapWidget::zoomToGpxBox()
{
    centerOn(m_gpxBox);
    m_gpxBox.clear();
}

KGeoTag::Coordinates MapWidget::currentCenter() const
{
    const auto center = focusPoint();
    return KGeoTag::Coordinates { center.longitude(Marble::GeoDataCoordinates::Degree),
                                  center.latitude(Marble::GeoDataCoordinates::Degree),
                                  0.0, true };
}
