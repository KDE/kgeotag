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
#include "ImagesListView.h"
#include "SharedObjects.h"
#include "ImagesModel.h"
#include "Settings.h"
#include "ImagesViewFilter.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QMouseEvent>
#include <QApplication>
#include <QMimeData>
#include <QDrag>
#include <QDebug>
#include <QKeyEvent>
#include <QMenu>

// C++ includes
#include <algorithm>
#include <functional>

ImagesListView::ImagesListView(KGeoTag::ImagesListType type, SharedObjects *sharedObjects,
                               QWidget *parent)
    : QListView(parent),
      m_type(type),
      m_bookmarks(sharedObjects->bookmarks())
{
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DropOnly);

    auto *filterModel = new ImagesViewFilter(this, type);
    filterModel->setSourceModel(sharedObjects->imagesModel());
    setModel(filterModel);
    connect(filterModel, &ImagesViewFilter::requestAddingImages,
            this, &ImagesListView::requestAddingImages);
    connect(filterModel, &ImagesViewFilter::requestRemoveCoordinates,
            this, QOverload<const QVector<QString> &>::of(&ImagesListView::removeCoordinates));

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    const int iconSize = sharedObjects->settings()->thumbnailSize();
    setIconSize(QSize(iconSize, iconSize));

    connect(this, &QAbstractItemView::clicked, this, &ImagesListView::centerImage);
    connect(this, &QAbstractItemView::clicked, this, &ImagesListView::imageSelected);

    // Context menu

    m_contextMenu = new QMenu(this);

    if (m_type != KGeoTag::AllImages) {
        m_selectAll = m_contextMenu->addAction(i18n("Select all images"));
        connect(m_selectAll, &QAction::triggered, this, &QListView::selectAll);
    } else {
        m_selectMenu = m_contextMenu->addMenu(i18n("Select"));
        auto *all = m_selectMenu->addAction(i18n("All images"));
        connect(all, &QAction::triggered, this, &QListView::selectAll);
        auto *without = m_selectMenu->addAction(i18n("All images without coordinates"));
        connect(without, &QAction::triggered,
                this, std::bind(&ImagesListView::selectImages, this, false));
        auto *with = m_selectMenu->addAction(i18n("All images with coordinates"));
        connect(with, &QAction::triggered,
                this, std::bind(&ImagesListView::selectImages, this, true));
    }

    m_contextMenu->addSeparator();

    m_automaticMatchingMenu = m_contextMenu->addMenu(i18n("Automatic matching"));

    auto *combinedMatchSearchAction = m_automaticMatchingMenu->addAction(
        i18n("Combined match search"));
    connect(combinedMatchSearchAction, &QAction::triggered,
            this, std::bind(&ImagesListView::requestAutomaticMatching, this,
                            this, KGeoTag::CombinedMatchSearch));

    m_automaticMatchingMenu->addSeparator();

    auto *searchExactMatchesAction = m_automaticMatchingMenu->addAction(
        i18n("Search exact matches only"));
    connect(searchExactMatchesAction, &QAction::triggered,
            this, std::bind(&ImagesListView::requestAutomaticMatching, this,
                            this, KGeoTag::ExactMatchSearch));

    auto *searchInterpolatedMatchesAction = m_automaticMatchingMenu->addAction(
        i18n("Search interpolated matches only"));
    connect(searchInterpolatedMatchesAction, &QAction::triggered,
            this, std::bind(&ImagesListView::requestAutomaticMatching, this,
                            this, KGeoTag::InterpolatedMatchSearch));

    m_bookmarksMenu = m_contextMenu->addMenu(i18n("Assign to bookmark"));
    updateBookmarks();

    m_contextMenu->addSeparator();

    m_assignToMapCenter = m_contextMenu->addAction(i18n("Assign to map center"));
    connect(m_assignToMapCenter, &QAction::triggered,
            this, std::bind(&ImagesListView::assignToMapCenter, this, this));

    m_assignManually = m_contextMenu->addAction(i18n("Set coordinates manually"));
    connect(m_assignManually, &QAction::triggered,
            this, std::bind(&ImagesListView::assignManually, this, this));

    m_editCoordinates = m_contextMenu->addAction(i18n("Edit coordinates"));
    connect(m_editCoordinates, &QAction::triggered,
            this, std::bind(&ImagesListView::editCoordinates, this, this));

    m_lookupElevation = m_contextMenu->addAction(i18n("Lookup elevation"));
    connect(m_lookupElevation, &QAction::triggered,
            this, std::bind(&ImagesListView::lookupElevation, this, this));

    m_contextMenu->addSeparator();

    m_removeCoordinates = m_contextMenu->addAction(i18n("Remove coordinates"));
    connect(m_removeCoordinates, &QAction::triggered,
            this, std::bind(QOverload<ImagesListView *>::of(&ImagesListView::removeCoordinates),
                            this, this));

    m_contextMenu->addSeparator();

    m_discardChanges = m_contextMenu->addAction(i18n("Discard changes"));
    connect(m_discardChanges, &QAction::triggered,
            this, std::bind(&ImagesListView::discardChanges, this, this));

    connect(this, &QListView::customContextMenuRequested, this, &ImagesListView::showContextMenu);
}

void ImagesListView::currentChanged(const QModelIndex &current, const QModelIndex &)
{
    if (current.isValid()) {
        emit imageSelected(current);
    }
}

void ImagesListView::updateBookmarks()
{
    m_bookmarksMenu->clear();
    auto bookmarks = m_bookmarks->keys();

    if (bookmarks.count() == 0) {
        m_bookmarksMenu->addAction(i18n("(No bookmarks defined)"));
        return;
    }

    std::sort(bookmarks.begin(), bookmarks.end());
    for (const auto &label : std::as_const(bookmarks)) {
        auto *entry = m_bookmarksMenu->addAction(label);
        entry->setData(label);
        connect(entry, &QAction::triggered,
                this, std::bind([this](QAction *action)
                {
                    emit assignTo(selectedPaths(), m_bookmarks->value(action->data().toString()));
                },
                entry));
    }
}

void ImagesListView::mousePressEvent(QMouseEvent *event)
{
    const auto pos = event->pos();
    const auto selectedIndex = indexAt(pos);
    if (event->button() == Qt::LeftButton && selectedIndex.isValid()) {
        m_dragStarted = true;
        m_dragStartPosition = event->pos();
    } else {
        m_dragStarted = false;
    }

    QListView::mousePressEvent(event);
}

void ImagesListView::mouseMoveEvent(QMouseEvent *event)
{
    // Enable selecting more images by dragging when the shift key is pressed
    if ((event->buttons() & Qt::LeftButton) && event->modifiers() == Qt::ShiftModifier) {
        QListView::mouseMoveEvent(event);
        return;
    }

    if (! (event->buttons() & Qt::LeftButton)
        || ! m_dragStarted
        || (event->pos() - m_dragStartPosition).manhattanLength()
           < QApplication::startDragDistance()) {

        return;
    }

    auto *drag = new QDrag(this);
    const auto paths = selectedPaths();

    if (paths.count() == 1) {
        const auto image = currentIndex().data(KGeoTag::ThumbnailRole);
        drag->setPixmap(QPixmap::fromImage(image.value<QImage>()));
    }

    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    for (const auto &path : paths) {
        urls.append(QUrl::fromLocalFile(path));
    }
    mimeData->setUrls(urls);

    mimeData->setData(KGeoTag::SourceImagesListMimeType,
                      KGeoTag::SourceImagesList.value(m_type));

    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}

QVector<QString> ImagesListView::selectedPaths() const
{
    QVector<QString> paths;
    const auto selected = selectedIndexes();
    for (const auto &index : selected) {
        paths.append(index.data(KGeoTag::PathRole).toString());
    }
    return paths;
}

void ImagesListView::keyPressEvent(QKeyEvent *event)
{
    QListView::keyPressEvent(event);

    const auto key = event->key();
    if (! (key == Qt::Key_Up || key == Qt::Key_Down
           || key == Qt::Key_PageUp || key == Qt::Key_PageDown)) {

        return;
    }

    emit centerImage(currentIndex());
}

void ImagesListView::showContextMenu(const QPoint &point)
{
    const auto selected = selectedIndexes();
    const int allSelected = selected.count();
    const bool anySelected = allSelected > 0;

    if (m_type == KGeoTag::AllImages) {
        m_selectMenu->setEnabled(model()->rowCount() > 0);
    } else {
        m_selectAll->setEnabled(model()->rowCount() > 0);
    }

    m_automaticMatchingMenu->setEnabled(anySelected);
    m_bookmarksMenu->setEnabled(anySelected);
    m_assignToMapCenter->setEnabled(anySelected);
    m_assignManually->setEnabled(anySelected);
    m_editCoordinates->setEnabled(anySelected);
    m_lookupElevation->setEnabled(anySelected);
    m_removeCoordinates->setEnabled(anySelected);
    m_discardChanges->setEnabled(anySelected);

    int hasCoordinates = 0;
    int changed = 0;

    for (const auto &index : selected) {
        if (index.data(KGeoTag::CoordinatesRole).value<Coordinates>().isSet()) {
            hasCoordinates++;
        }

        if (index.data(KGeoTag::ChangedRole).toBool()) {
            changed++;
        }
    }

    m_assignManually->setVisible(hasCoordinates == 0);
    m_editCoordinates->setVisible(hasCoordinates > 0);
    m_lookupElevation->setVisible(hasCoordinates == allSelected);
    m_removeCoordinates->setVisible(hasCoordinates > 0);
    m_discardChanges->setVisible(changed > 0);

    m_contextMenu->exec(mapToGlobal(point));
}

void ImagesListView::selectImages(bool coordinatesSet)
{
    clearSelection();
    for (int i = 0; i < model()->rowCount(); i++) {
        const auto index = model()->index(i, 0);
        if (index.data(KGeoTag::CoordinatesRole).value<Coordinates>().isSet() == coordinatesSet) {
            selectionModel()->select(index, QItemSelectionModel::Select);
        }
    }
}
