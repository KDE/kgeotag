// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef BOOKMARKSWIDGET_H
#define BOOKMARKSWIDGET_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// Qt includes
#include <QWidget>

// Local classes
class SharedObjects;
class BookmarksList;
class CoordinatesFormatter;

// Qt classes
class QLabel;

class BookmarksWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BookmarksWidget(SharedObjects *sharedObjects, QWidget *parent = nullptr);
    const QHash<QString, Coordinates> *bookmarks() const;

Q_SIGNALS:
    void bookmarksChanged();
    void requestAddBookmark();

private Q_SLOTS:
    void showInfo(const Coordinates &coordinates);

private: // Variables
    CoordinatesFormatter *m_formatter;
    BookmarksList *m_bookmarksList;
    QLabel *m_info;

};

#endif // BOOKMARKSWIDGET_H
