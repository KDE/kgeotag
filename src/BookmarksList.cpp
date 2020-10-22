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

BookmarksList::BookmarksList(Settings *settings, MapWidget *mapWidget, QWidget *parent)
    : QListWidget(parent),
      m_settings(settings),
      m_mapWidget(mapWidget)
{
    setSortingEnabled(true);

    m_contextMenu = new QMenu(this);
    auto *newBookmarkAction = m_contextMenu->addAction(i18n("Add new bookmark for current map "
                                                            "center"));
    connect(newBookmarkAction, &QAction::triggered, this, &BookmarksList::newBookmark);

    m_contextMenu->addSeparator();

    m_renameBookmark = m_contextMenu->addAction(i18n("Rename bookmark"));
    connect(m_renameBookmark, &QAction::triggered, this, &BookmarksList::renameBookmark);

    m_deleteBookmark = m_contextMenu->addAction(i18n("Delete bookmark"));
    connect(m_deleteBookmark, &QAction::triggered, this, &BookmarksList::deleteBookmark);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &BookmarksList::showContextMenu);

    connect(this, &QListWidget::currentItemChanged, this, &BookmarksList::itemHighlighted);
}

void BookmarksList::showContextMenu(const QPoint &point)
{
    const bool itemSelected = currentItem() != nullptr;
    m_renameBookmark->setEnabled(itemSelected);
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

    auto *item = new QListWidgetItem(label);
    item->setData(ItemData::Lon, coordinates.lon);
    item->setData(ItemData::Lat, coordinates.lat);
    addItem(item);
    setCurrentItem(item);
}

void BookmarksList::itemHighlighted(QListWidgetItem *item, QListWidgetItem *)
{
    if (item == nullptr) {
        return;
    }

    const KGeoTag::Coordinates coordinates { item->data(ItemData::Lon).toDouble(),
                                             item->data(ItemData::Lat).toDouble(),
                                             0.0, true };

    m_mapWidget->centerCoordinates(coordinates);
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
    auto *item = currentItem();

    const auto currentLabel = item->text();
    auto [ label, okay ] = getString(i18n("Rename Bookmark"),
                                     i18n("New label for the new bookmark:"), currentLabel);
    if (! okay || label == currentLabel) {
        return;
    }

    label = label.simplified();
    if (! findItems(label, Qt::MatchExactly).isEmpty()) {
        QMessageBox::warning(this, i18n("Rename Bookmark"),
                             i18n("The label \"%1\" is already in use. Please choose another "
                                  "name.", label));
        return;
    }

    item->setText(label);
}

void BookmarksList::deleteBookmark()
{
    if (QMessageBox::question(this, i18n("Delete bookmark"),
        i18n("Really delete bookmark \"%1\"?", currentItem()->text()),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) {

        return;
    }

    const auto *item = takeItem(currentRow());
    delete item;
}
