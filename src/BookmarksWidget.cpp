// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
    connect(this, &BookmarksWidget::requestAddBookmark,
            m_bookmarksList, &BookmarksList::newBookmark);
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
        m_info->setText(i18n("Position: %1; Altitude: %2",
                             m_formatter->format(coordinates),
                             m_formatter->alt(coordinates)));
    }
}
