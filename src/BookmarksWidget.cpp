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
#include "KGeoTag.h"
#include "BookmarksList.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QLabel>

BookmarksWidget::BookmarksWidget(SharedObjects *sharedObjects, QWidget *parent)
    : QWidget(parent),
      m_settings(sharedObjects->settings())
{
    auto *layout = new QVBoxLayout(this);

    auto *bookmarks = new BookmarksList(sharedObjects);
    connect(bookmarks, &BookmarksList::showInfo, this, &BookmarksWidget::showInfo);
    layout->addWidget(bookmarks);

    m_info = new QLabel;
    layout->addWidget(m_info);
}

void BookmarksWidget::showInfo(const QString &id)
{
    const auto coordinates = m_settings->bookmarkCoordinates(id);
    m_info->setText(i18n("Position: %1° %2, %3° %4; Altitude: %5 m",
        std::abs(coordinates.lon), coordinates.lon >= 0 ? i18nc("Cardinal direction", "E")
                                                        : i18nc("Cardinal direction", "W"),
        std::abs(coordinates.lat), coordinates.lat >= 0 ? i18nc("Cardinal direction", "N")
                                                        : i18nc("Cardinal direction", "S"),
    coordinates.alt));
}
