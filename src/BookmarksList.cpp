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
#include "BookmarksList.h"
#include "SharedObjects.h"
#include "Settings.h"
#include "MapWidget.h"

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
    connect(m_elevationEngine, &ElevationEngine::elevationProcessed,
            this, &BookmarksList::elevationProcessed);
    connect(m_elevationEngine, &ElevationEngine::lookupFailed,
            this, &BookmarksList::restoreAfterElevationLookup);

    setSortingEnabled(true);

    m_contextMenu = new QMenu(this);
    auto *newBookmarkAction = m_contextMenu->addAction(i18n("Add new bookmark for current map "
                                                            "center"));
    connect(newBookmarkAction, &QAction::triggered, this, &BookmarksList::newBookmark);

    m_contextMenu->addSeparator();

    m_renameBookmark = m_contextMenu->addAction(i18n("Rename bookmark"));
    connect(m_renameBookmark, &QAction::triggered, this, &BookmarksList::renameBookmark);

    m_lookupElevation = m_contextMenu->addAction(i18n("Fetch elevation"));
    connect(m_lookupElevation, &QAction::triggered, this, &BookmarksList::lookupElevation);

    m_setElevation = m_contextMenu->addAction(i18n("Set elevation manually"));
    connect(m_setElevation, &QAction::triggered, this, &BookmarksList::setElevation);

    m_contextMenu->addSeparator();

    m_deleteBookmark = m_contextMenu->addAction(i18n("Delete bookmark"));
    connect(m_deleteBookmark, &QAction::triggered, this, &BookmarksList::deleteBookmark);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &BookmarksList::showContextMenu);

    connect(this, &QListWidget::currentItemChanged, this, &BookmarksList::itemHighlighted);
}

void BookmarksList::showContextMenu(const QPoint &point)
{
    m_contextMenuItem = itemAt(point);

    const bool itemSelected = m_contextMenuItem != nullptr;
    m_renameBookmark->setEnabled(itemSelected);
    m_lookupElevation->setEnabled(itemSelected && m_settings->lookupElevation());
    m_setElevation->setEnabled(itemSelected);
    m_deleteBookmark->setEnabled(itemSelected);

    m_contextMenu->exec(mapToGlobal(point));
}

void BookmarksList::newBookmark()
{
    auto [ label, okay ] = getString(i18n("New Bookmark"), i18n("Label for the new bookmark:"));
    if (! okay) {
        return;
    }

    label = label.simplified();
    if (label.isEmpty()) {
        label = i18n("Untitled");
    }

    QString searchLabel = label;
    int addition = 0;
    while (! findItems(searchLabel, Qt::MatchExactly).isEmpty()) {
        addition++;
        searchLabel = i18nc("Adding a consecutive number to a bookmark label to make it unique",
                            "%1 (%2)", label, addition);
    }
    label = searchLabel;

    const auto coordinates = m_mapWidget->currentCenter();
    m_bookmarks[label] = coordinates;

    auto *item = new QListWidgetItem(label);
    addItem(item);
    setCurrentItem(item);

    if (m_settings->lookupElevation()) {
        requestElevation(label);
    }
}

void BookmarksList::requestElevation(const QString &id)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setEnabled(false);
    m_elevationEngine->request(ElevationEngine::Target::Bookmark, id, m_bookmarks.value(id));
}

void BookmarksList::itemHighlighted(QListWidgetItem *item, QListWidgetItem *)
{
    if (item == nullptr) {
        emit showInfo(KGeoTag::NoCoordinates);
        return;
    }

    const QString id = item->text();
    const auto coordinates = m_bookmarks.value(id);
    m_mapWidget->centerCoordinates(coordinates);
    emit showInfo(coordinates);
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
}

void BookmarksList::elevationProcessed(ElevationEngine::Target target, const QString &id,
                                       double elevation)
{
    if (target != ElevationEngine::Target::Bookmark) {
        return;
    }

    restoreAfterElevationLookup();
    m_bookmarks[id].alt = elevation;
    emit showInfo(m_bookmarks.value(id));
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
                                             i18n("Elevation for \"%1\" (m)", id), 0, -12000, 8900,
                                             1, &okay);
    if (! okay) {
        return;
    }

    elevationProcessed(ElevationEngine::Target::Bookmark, id, elevation);
}
