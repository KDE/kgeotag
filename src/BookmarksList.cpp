// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "BookmarksList.h"
#include "SharedObjects.h"
#include "Settings.h"
#include "MapWidget.h"
#include "CoordinatesDialog.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QDebug>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QApplication>

BookmarksList::BookmarksList(SharedObjects *sharedObjects, QWidget *parent)
    : QListWidget(parent),
      m_settings(sharedObjects->settings()),
      m_elevationEngine(sharedObjects->elevationEngine()),
      m_mapWidget(sharedObjects->mapWidget())
{
    setSortingEnabled(true);

    // Load the saved bookmarks
    m_bookmarks = m_settings->bookmarks();
    const auto labels = m_bookmarks.keys();
    for (const auto &label : labels) {
        auto *item = new QListWidgetItem(label);
        addItem(item);
    }

    // Show the first bookmark's coordinates if we have bookmarks
    if (! m_bookmarks.isEmpty()) {
        setCurrentItem(0);
    }

    connect(this, &QListWidget::itemClicked, this, &BookmarksList::centerBookmark);
    connect(this, &QListWidget::currentItemChanged, this, &BookmarksList::itemHighlighted);

    connect(m_elevationEngine, &ElevationEngine::elevationProcessed,
            this, &BookmarksList::elevationProcessed);
    connect(m_elevationEngine, &ElevationEngine::lookupFailed,
            this, &BookmarksList::restoreAfterElevationLookup);

    // Context menu

    m_contextMenu = new QMenu(this);

    auto *newBookmarkAction = m_contextMenu->addAction(i18n("Add new bookmark for current map "
                                                            "center"));
    connect(newBookmarkAction, &QAction::triggered, this, &BookmarksList::newBookmark);

    auto *newManualBookmarkAction = m_contextMenu->addAction(i18n("Add new manual bookmark"));
    connect(newManualBookmarkAction, &QAction::triggered, this, &BookmarksList::newManualBookmark);

    m_contextMenu->addSeparator();

    m_renameBookmark = m_contextMenu->addAction(i18n("Rename bookmark"));
    connect(m_renameBookmark, &QAction::triggered, this, &BookmarksList::renameBookmark);

    m_editCoordinates = m_contextMenu->addAction(i18n("Edit coordinates"));
    connect(m_editCoordinates, &QAction::triggered, this, &BookmarksList::editCoordinates);

    m_contextMenu->addSeparator();

    m_lookupElevation = m_contextMenu->addAction(i18n("Lookup elevation"));
    connect(m_lookupElevation, &QAction::triggered, this, &BookmarksList::lookupElevation);

    m_setElevation = m_contextMenu->addAction(i18n("Set elevation manually"));
    connect(m_setElevation, &QAction::triggered, this, &BookmarksList::setElevation);

    m_contextMenu->addSeparator();

    m_deleteBookmark = m_contextMenu->addAction(i18n("Delete bookmark"));
    connect(m_deleteBookmark, &QAction::triggered, this, &BookmarksList::deleteBookmark);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &BookmarksList::showContextMenu);
}

void BookmarksList::showContextMenu(const QPoint &point)
{
    m_contextMenuItem = itemAt(point);

    const bool itemSelected = m_contextMenuItem != nullptr;
    m_renameBookmark->setEnabled(itemSelected);
    m_lookupElevation->setEnabled(itemSelected);
    m_setElevation->setEnabled(itemSelected);
    m_deleteBookmark->setEnabled(itemSelected);
    m_editCoordinates->setEnabled(itemSelected);

    m_contextMenu->exec(mapToGlobal(point));
}

void BookmarksList::newBookmark()
{
    auto [ label, okay ] = getString(i18n("New Bookmark"), i18n("Label for the new bookmark:"));
    if (! okay) {
        return;
    }

    label = label.simplified();
    const auto originalLabel = label;
    if (label.isEmpty()) {
        label = i18n("Untitled");
    }

    saveBookmark(label, m_mapWidget->currentCenter());
}

void BookmarksList::newManualBookmark()
{
    CoordinatesDialog dialog(CoordinatesDialog::Mode::ManualBookmark,
                             m_settings->lookupElevationAutomatically(),
                             m_settings->latBeforeLon());
    if (! dialog.exec()) {
        return;
    }

    saveBookmark(dialog.label(),
                 Coordinates(dialog.lon(), dialog.lat(), dialog.alt(), true));
}

void BookmarksList::saveBookmark(QString label, const Coordinates &coordinates)
{
    const QString originalLabel = label;
    QString searchLabel = label;
    int addition = 0;
    while (! findItems(searchLabel, Qt::MatchExactly).isEmpty()) {
        addition++;
        searchLabel = i18nc("Adding a consecutive number to a bookmark label to make it unique",
                            "%1 (%2)", label, addition);
    }
    label = searchLabel;

    m_bookmarks[label] = coordinates;

    auto *item = new QListWidgetItem(label);
    addItem(item);
    setCurrentItem(item);

    if (m_settings->lookupElevationAutomatically()) {
        requestElevation(label);
    }

    Q_EMIT bookmarksChanged();

    if (label != originalLabel) {
        QMessageBox::warning(this, i18n("Add new bookmark"),
            i18n("The bookmark \"%1\" already exists, created \"%2\" instead.",
                 originalLabel, label));
    }

    m_mapWidget->centerCoordinates(coordinates);
}

void BookmarksList::requestElevation(const QString &id)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setEnabled(false);
    m_elevationEngine->request(ElevationEngine::Target::Bookmark, { id },
                               { m_bookmarks.value(id) });
}

void BookmarksList::itemHighlighted(QListWidgetItem *item, QListWidgetItem *)
{
    if (item == nullptr) {
        Q_EMIT showInfo(Coordinates());
        return;
    }

    Q_EMIT showInfo(m_bookmarks.value(item->text()));
}

void BookmarksList::centerBookmark(QListWidgetItem *item)
{
    m_mapWidget->centerCoordinates(m_bookmarks.value(item->text()));
}

BookmarksList::EnteredString BookmarksList::getString(const QString &title, const QString &label,
                                                      const QString &text)
{
    bool okay = false;
    auto string = QInputDialog::getText(this, title, label, QLineEdit::Normal, text, &okay);
    return { string, okay };
}

void BookmarksList::renameBookmark()
{
    const auto currentLabel = m_contextMenuItem->text();
    auto [ newLabel, okay ] = getString(i18n("Rename Bookmark"),
                                        i18n("New label for the new bookmark:"), currentLabel);
    if (! okay || newLabel == currentLabel) {
        return;
    }

    newLabel = newLabel.simplified();
    if (! findItems(newLabel, Qt::MatchExactly).isEmpty()) {
        QMessageBox::warning(this, i18n("Rename Bookmark"),
                             i18n("The label \"%1\" is already in use. Please choose another "
                                  "name.", newLabel));
        return;
    }

    m_bookmarks[newLabel] = m_bookmarks.value(currentLabel);
    m_bookmarks.remove(currentLabel);
    m_contextMenuItem->setText(newLabel);

    Q_EMIT bookmarksChanged();
}

void BookmarksList::deleteBookmark()
{
    if (QMessageBox::question(this, i18n("Delete bookmark"),
        i18n("Really delete bookmark \"%1\"?", m_contextMenuItem->text()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {

        return;
    }

    m_bookmarks.remove(m_contextMenuItem->text());
    const auto *item = takeItem(row(m_contextMenuItem));
    delete item;

    Q_EMIT bookmarksChanged();
}

void BookmarksList::elevationProcessed(ElevationEngine::Target target, const QVector<QString> &ids,
                                       const QVector<double> &elevations)
{
    if (target != ElevationEngine::Target::Bookmark) {
        return;
    }

    restoreAfterElevationLookup();
    const auto id = ids.at(0);
    m_bookmarks[id].setAlt(elevations.at(0));
    Q_EMIT showInfo(m_bookmarks.value(id));
}

void BookmarksList::restoreAfterElevationLookup()
{
    if (! isEnabled()) {
        QApplication::restoreOverrideCursor();
        setEnabled(true);
    }
}

void BookmarksList::lookupElevation()
{
    requestElevation(m_contextMenuItem->text());
}

void BookmarksList::setElevation()
{
    const auto id = m_contextMenuItem->text();

    bool okay = false;
    auto elevation = QInputDialog::getDouble(this, i18n("Set elevation"),
                                             i18n("Elevation for \"%1\" (m)", id),
                                             m_bookmarks.value(id).alt(), KGeoTag::minimalAltitude,
                                             KGeoTag::maximalAltitude, 1, &okay);
    if (! okay) {
        return;
    }

    elevationProcessed(ElevationEngine::Target::Bookmark, { id }, { elevation });
}

const QHash<QString, Coordinates> *BookmarksList::bookmarks() const
{
    return &m_bookmarks;
}

void BookmarksList::editCoordinates()
{
    const auto id = m_contextMenuItem->text();
    auto &coordinates = m_bookmarks[id];

    CoordinatesDialog dialog(CoordinatesDialog::Mode::EditCoordinates, false,
                             m_settings->latBeforeLon(), coordinates,
                             i18nc("A quoted bookmark label", "\"%1\"", id));
    if (! dialog.exec()) {
        return;
    }

    coordinates = dialog.coordinates();
    m_mapWidget->centerCoordinates(coordinates);
    Q_EMIT showInfo(coordinates);
}
