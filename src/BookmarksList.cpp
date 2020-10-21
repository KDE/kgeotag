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

BookmarksList::BookmarksList(Settings *settings, MapWidget *mapWidget, QWidget *parent)
    : QListWidget(parent),
      m_settings(settings),
      m_mapWidget(mapWidget)
{
    m_contextMenu = new QMenu(this);
    auto *newBookmarkAction = m_contextMenu->addAction(i18n("Add new bookmark for current map "
                                                            "center"));
    connect(newBookmarkAction, &QAction::triggered, this, &BookmarksList::newBookmark);

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, &BookmarksList::showContextMenu);
}

void BookmarksList::showContextMenu(const QPoint &point)
{
    m_contextMenu->exec(mapToGlobal(point));
}

void BookmarksList::newBookmark()
{
    bool okay = false;
    auto label = QInputDialog::getText(this, i18n("New Bookmark"),
                                       i18n("Label for the new bookmark:"), QLineEdit::Normal,
                                       QString(), &okay);
    if (! okay) {
        return;
    }

    if (label.isEmpty()) {
        label = i18n("Untitled");
    }

    const auto coordinates = m_mapWidget->currentCenter();
    qDebug() << label << coordinates.lon << coordinates.lat;
}
