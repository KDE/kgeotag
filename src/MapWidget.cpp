// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "MapWidget.h"
#include "SharedObjects.h"
#include "TracksLayer.h"
#include "ImagesLayer.h"
#include "Settings.h"
#include "KGeoTag.h"
#include "ImagesModel.h"
#include "MimeHelper.h"
#include "GeoDataModel.h"
#include "CoordinatesFormatter.h"

// Marble includes
#include <marble/GeoPainter.h>
#include <marble/AbstractFloatItem.h>
#include <marble/MarbleWidgetInputHandler.h>
#include <marble/ViewportParams.h>
#include <marble/GeoDataLatLonAltBox.h>

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
#include <QGuiApplication>
#include <QClipboard>
#include <QMessageBox>

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
      m_geoDataModel(sharedObjects->geoDataModel()),
      m_imagesModel(sharedObjects->imagesModel()),
      m_coordinatesFormatter(sharedObjects->coordinatesFormatter())
{
    connect(this, &Marble::MarbleWidget::visibleLatLonAltBoxChanged,
            this, [this]
            {
                Q_EMIT mapMoved(currentCenter());
            });

    setAcceptDrops(true);

    setProjection(Marble::Mercator);
    setMapThemeId(QStringLiteral("earth/openstreetmap/openstreetmap.dgml"));

    auto *tracksLayer = new TracksLayer(this, m_geoDataModel, &m_trackPen);
    auto *imagesLayer = new ImagesLayer(this, m_imagesModel);
    addLayer(tracksLayer);
    addLayer(imagesLayer);

    m_trackPen.setCapStyle(Qt::RoundCap);
    m_trackPen.setJoinStyle(Qt::RoundJoin);
    updateSettings();

    // Build a context menu

    m_contextMenu = new QMenu(this);

    auto *floatersMenu = m_contextMenu->addMenu(i18n("Displayed floating items"));

    const auto floatItemsList = floatItems();
    const auto visibility = m_settings->floatersVisibility();

    // Add a "Toggle crosshairs" action
    auto *crossHairsAction = floatersMenu->addAction(i18n("Crosshairs"));
    crossHairsAction->setCheckable(true);
    connect(crossHairsAction, &QAction::toggled, this, &Marble::MarbleWidget::setShowCrosshairs);
    crossHairsAction->setChecked(m_settings->showCrosshairs());
    setShowCrosshairs(m_settings->showCrosshairs());

    // Add actions for all supported floaters

    floatersMenu->addSeparator();

    for (const auto &item : floatItemsList) {
        const auto id = item->nameId();
        if (s_unsupportedFloaters.contains(id)) {
            auto item = floatItem(id);
            if (item != nullptr) {
                item->setVisible(false);
            }
            continue;
        }

        auto *action = floatersMenu->addAction(item->name());
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
        floatersMenu->addSeparator();
        auto *licenseAction = floatersMenu->addAction(licenseFloater->name());
        licenseAction->setCheckable(true);
        licenseAction->setChecked(true);
        licenseAction->setData(s_licenseFloaterId);
        connect(licenseAction, &QAction::toggled,
                this, std::bind(&MapWidget::changeFloaterVisiblity, this, licenseAction));
        m_floatersActions.append(licenseAction);
    }

    // Map center actions

    m_contextMenu->addSeparator();
    m_mapCenterMenu = m_contextMenu->addMenu(i18n("Current map center"));

    // Copy coordinates to clipboard
    auto *copyAction = m_mapCenterMenu->addAction(i18n("Copy coordinates to clipboard"));
    connect(copyAction, &QAction::triggered, this, [this]
            {
                QGuiApplication::clipboard()->setText(
                    m_coordinatesFormatter->format(currentCenter()));
                QMessageBox::information(this, i18n("Copy coordinates to clipboard"),
                                         i18n("Coordinates copied!"));
            });

    // Request adding a bookmark
    m_mapCenterMenu->addSeparator();
    auto *requestBookmarkAction = m_mapCenterMenu->addAction(i18n("Add bookmark"));
    connect(requestBookmarkAction, &QAction::triggered, this, &MapWidget::requestAddBookmark);

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

QMenu *MapWidget::mapCenterMenu() const
{
    return m_mapCenterMenu;
}

void MapWidget::changeFloaterVisiblity(QAction *action)
{
    floatItem(action->data().toString())->setVisible(action->isChecked());
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
    // Restore the floaters visibility
    const auto floatersVisibility = m_settings->floatersVisibility();
    const auto floatItemsList = floatItems();
    for (const auto &item : floatItemsList) {
        const auto name = item->nameId();
        if (floatersVisibility.contains(name)) {
            item->setVisible(floatersVisibility.value(name));
        }
    }

    // Restore map's last center point
    const auto center = m_settings->mapCenter();
    centerOn(center.lon(), center.lat());

    // Restore the last zoom level
    setZoom(m_settings->zoom());
}

void MapWidget::dragEnterEvent(QDragEnterEvent *event)
{
    const auto mimeData = event->mimeData();
    const auto source = mimeData->data(KGeoTag::SourceImagesListMimeType);

    if (! source.isEmpty()) {
        // The drag most probably comes from a KGeoTag images list.
        // Nevertheless, we check if all URLs are present in the images model ;-)
        const auto urls = mimeData->urls();
        for (const auto &url : urls) {
            if (! m_imagesModel->contains(url.toLocalFile())) {
                return;
            }
        }

    } else {
        // Possibly a request to load a GPX file
        if (MimeHelper::getUsablePaths(KGeoTag::DroppedOnMap, mimeData).isEmpty()) {
            return;
        }
    }

    event->acceptProposedAction();
}

void MapWidget::dropEvent(QDropEvent *event)
{
    const auto mimeData = event->mimeData();
    if (! mimeData->hasUrls()) {
        return;
    }

    const auto source = mimeData->data(KGeoTag::SourceImagesListMimeType);
    if (! source.isEmpty()) {
        // Images dragged from an images list

        const auto urls = mimeData->urls();
        QVector<QString> paths;

        // Be sure to really have all images
        for (const auto &url : urls) {
            const auto path = url.toLocalFile();
            if (m_imagesModel->contains(path)) {
                paths.append(path);
            }
        }

        // This should not happen ...
        if (paths.isEmpty()) {
            return;
        }

        const auto dropPosition = event->pos();

        qreal lon;
        qreal lat;
        if (! geoCoordinates(dropPosition.x(), dropPosition.y(), lon, lat,
                             Marble::GeoDataCoordinates::Degree)) {
            return;
        }

        for (const auto &path : paths) {
            m_imagesModel->setCoordinates(path, Coordinates(lon, lat, 0.0, true),
                                          KGeoTag::ManuallySet);
        }

        reloadMap();
        Q_EMIT imagesDropped(paths);

    } else {
        // Request to load a GPX file
        const auto usablePaths = MimeHelper::getUsablePaths(KGeoTag::DroppedOnMap, mimeData);
        if (! usablePaths.isEmpty()) {
            Q_EMIT requestLoadGpx(usablePaths);
        }
    }

    event->acceptProposedAction();
}

void MapWidget::centerImage(const QModelIndex &index)
{
    if (! index.isValid()) {
        return;
    }

    const auto coordinates = index.data(KGeoTag::CoordinatesRole).value<Coordinates>();
    if (coordinates.isSet()) {
        centerCoordinates(coordinates);
    }
}

void MapWidget::centerCoordinates(const Coordinates &coordinates)
{
    centerOn(coordinates.lon(), coordinates.lat());
}

void MapWidget::zoomToTrack(const QModelIndex &index)
{
    centerOn(m_geoDataModel->trackBox(index));
}

void MapWidget::zoomToTracks(const QVector<QString> &paths)
{
    Marble::GeoDataLatLonAltBox box;
    for (const auto &path : paths) {
        if (box.isEmpty()) {
            box = m_geoDataModel->trackBox(path);
        } else {
            box |= m_geoDataModel->trackBox(path);
        }
    }
    centerOn(box);
}

Coordinates MapWidget::currentCenter() const
{
    const auto center = focusPoint();
    return Coordinates(center.longitude(Marble::GeoDataCoordinates::Degree),
                       center.latitude(Marble::GeoDataCoordinates::Degree),
                       0.0,
                       true);
}
