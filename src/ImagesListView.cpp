// SPDX-FileCopyrightText: 2020-2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "ImagesListView.h"
#include "SharedObjects.h"
#include "ImagesModel.h"
#include "Settings.h"
#include "ImagesListFilter.h"

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
#include <QDesktopServices>

// C++ includes
#include <algorithm>
#include <functional>

ImagesListView::ImagesListView(KGeoTag::ImagesListType type, SharedObjects *sharedObjects,
                               QWidget *parent)
    : QListView(parent),
      m_listType(type),
      m_bookmarks(sharedObjects->bookmarks())
{
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DropOnly);

    m_listFilter = new ImagesListFilter(this, type);
    m_listFilter->setSourceModel(sharedObjects->imagesModel());
    setModel(m_listFilter);
    connect(m_listFilter, &ImagesListFilter::requestAddingImages,
            this, &ImagesListView::requestAddingImages);
    connect(m_listFilter, &ImagesListFilter::requestRemoveCoordinates,
            this, QOverload<const QVector<QString> &>::of(&ImagesListView::removeCoordinates));

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);
    const int iconSize = sharedObjects->settings()->thumbnailSize();
    setIconSize(QSize(iconSize, iconSize));

    connect(this, &QAbstractItemView::clicked, this, &ImagesListView::centerImage);
    connect(this, &QAbstractItemView::clicked, this, &ImagesListView::imageSelected);

    // Context menu

    m_contextMenu = new QMenu(this);

    m_selectAll = m_contextMenu->addAction(i18n("Select all images"));
    m_selectAll->setIcon(QIcon::fromTheme(QStringLiteral("select")));
    connect(m_selectAll, &QAction::triggered, this, &QListView::selectAll);

    m_selectMenu = m_contextMenu->addMenu(i18n("Select"));
    m_selectMenu->menuAction()->setIcon(QIcon::fromTheme(QStringLiteral("select")));
    auto *all = m_selectMenu->addAction(i18n("All images"));
    connect(all, &QAction::triggered, this, &QListView::selectAll);
    auto *without = m_selectMenu->addAction(i18n("All images without coordinates"));
    connect(without, &QAction::triggered,
            this, std::bind(&ImagesListView::selectImages, this, false));
    auto *with = m_selectMenu->addAction(i18n("All images with coordinates"));
    connect(with, &QAction::triggered,
            this, std::bind(&ImagesListView::selectImages, this, true));

    m_contextMenu->addSeparator();

    m_automaticMatchingMenu = m_contextMenu->addMenu(i18n("Automatic matching"));
    m_automaticMatchingMenu->menuAction()->setIcon(QIcon::fromTheme(QStringLiteral("run-build")));

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
    m_bookmarksMenu->menuAction()->setIcon(QIcon::fromTheme(QStringLiteral("bookmarks")));
    updateBookmarks();

    m_contextMenu->addSeparator();

    m_assignToMapCenter = m_contextMenu->addAction(i18n("Assign to map center"));
    m_assignToMapCenter->setIcon(QIcon::fromTheme(QStringLiteral("crosshairs")));
    connect(m_assignToMapCenter, &QAction::triggered,
            this, std::bind(&ImagesListView::assignToMapCenter, this, this));

    m_assignManually = m_contextMenu->addAction(i18n("Set coordinates manually"));
    m_assignManually->setIcon(QIcon::fromTheme(QStringLiteral("add-placemark")));
    connect(m_assignManually, &QAction::triggered,
            this, std::bind(&ImagesListView::assignManually, this, this));

    m_editCoordinates = m_contextMenu->addAction(i18n("Edit coordinates"));
    connect(m_editCoordinates, &QAction::triggered,
            this, std::bind(&ImagesListView::editCoordinates, this, this));

    m_lookupElevation = m_contextMenu->addAction(i18n("Lookup elevation"));
    m_lookupElevation->setIcon(QIcon::fromTheme(QStringLiteral("adjustcurves")));
    connect(m_lookupElevation, &QAction::triggered,
            this, std::bind(&ImagesListView::lookupElevation, this, this));

    m_contextMenu->addSeparator();

    m_save = m_contextMenu->addAction(i18n("Save changes"));
    m_save->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    connect(m_save, &QAction::triggered,
            this, std::bind(&ImagesListView::requestSaving, this, this));

    m_contextMenu->addSeparator();

    m_removeCoordinates = m_contextMenu->addAction(i18n("Remove coordinates"));
    connect(m_removeCoordinates, &QAction::triggered,
            this, std::bind(QOverload<ImagesListView *>::of(&ImagesListView::removeCoordinates),
                            this, this));

    m_contextMenu->addSeparator();

    m_discardChanges = m_contextMenu->addAction(i18n("Discard changes"));
    m_discardChanges->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")));
    connect(m_discardChanges, &QAction::triggered,
            this, std::bind(&ImagesListView::discardChanges, this, this));

    m_removeImages = m_contextMenu->addAction(i18np("Remove image", "Remove images", 1));
    m_removeImages->setIcon(QIcon::fromTheme(QStringLiteral("document-close")));
    connect(m_removeImages, &QAction::triggered,
            this, std::bind(&ImagesListView::removeImages, this, this));

    m_contextMenu->addSeparator();

    m_openExternally = m_contextMenu->addAction(i18n("Open with default viewer"));
    m_openExternally->setIcon(QIcon::fromTheme(QStringLiteral("file-zoom-in")));
    connect(m_openExternally, &QAction::triggered, this, &ImagesListView::openExternally);

    connect(this, &QListView::customContextMenuRequested, this, &ImagesListView::showContextMenu);
}

void ImagesListView::setListType(KGeoTag::ImagesListType type)
{
    m_listType = type;
    m_listFilter->setListType(type);
    m_selectAll->setVisible(type != KGeoTag::AllImages);
    m_selectMenu->menuAction()->setVisible(type == KGeoTag::AllImages);
}

void ImagesListView::currentChanged(const QModelIndex &current, const QModelIndex &)
{
    if (current.isValid()) {
        emit imageSelected(current);
        scrollTo(current);
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
        drag->setPixmap(currentIndex().data(KGeoTag::ThumbnailRole).value<QPixmap>());
    }

    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    for (const auto &path : paths) {
        urls.append(QUrl::fromLocalFile(path));
    }
    mimeData->setUrls(urls);

    mimeData->setData(KGeoTag::SourceImagesListMimeType,
                      KGeoTag::SourceImagesList.value(m_listType));

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

    m_selectMenu->setEnabled(model()->rowCount() > 0);
    m_selectAll->setEnabled(model()->rowCount() > 0);

    m_automaticMatchingMenu->setEnabled(anySelected);
    m_bookmarksMenu->setEnabled(anySelected);
    m_assignToMapCenter->setEnabled(anySelected);
    m_assignManually->setEnabled(anySelected);
    m_editCoordinates->setEnabled(anySelected);
    m_lookupElevation->setEnabled(anySelected);
    m_removeCoordinates->setEnabled(anySelected);
    m_discardChanges->setEnabled(anySelected);
    m_removeImages->setEnabled(anySelected);
    if (anySelected) {
        m_removeImages->setText(i18np("Remove image", "Remove images", allSelected));
    }
    m_openExternally->setEnabled(allSelected == 1);

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
    m_save->setVisible(changed > 0);

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

void ImagesListView::openExternally()
{
    // selectedPaths() always contains exactly one entry when this is called,
    // because the connected action is only enabled in this very case
    QDesktopServices::openUrl(QUrl::fromLocalFile(selectedPaths().first()));
}
