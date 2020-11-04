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
#include "BookmarksWidget.h"
#include "SharedObjects.h"
#include "Settings.h"
#include "BookmarksList.h"
#include "CoordinatesFormatter.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QLabel>

BookmarksWidget::BookmarksWidget(SharedObjects *sharedObjects, QWidget *parent)
    : QWidget(parent),
      m_formatter(sharedObjects->coordinatesFormatter())
{
    auto *layout = new QVBoxLayout(this);

    m_bookmarksList = new BookmarksList(sharedObjects);
    connect(m_bookmarksList, &BookmarksList::bookmarksChanged,
            this, &BookmarksWidget::bookmarksChanged);
    connect(m_bookmarksList, &BookmarksList::showInfo, this, &BookmarksWidget::showInfo);
    layout->addWidget(m_bookmarksList);

    m_info = new QLabel;
    layout->addWidget(m_info);
}

const QHash<QString, Coordinates> *BookmarksWidget::bookmarks() const
{
    return m_bookmarksList->bookmarks();
}

void BookmarksWidget::showInfo(const Coordinates &coordinates)
{
    if (! coordinates.isSet()) {
        m_info->clear();
    } else {
        m_info->setText(i18n("Position: %1, %2; Altitude: %3 m",
                             m_formatter->lon(coordinates),
                             m_formatter->lat(coordinates),
                             m_formatter->alt(coordinates)));
    }
}
