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

#ifndef BOOKMARKSLIST_H
#define BOOKMARKSLIST_H

// Local includes
#include "ElevationEngine.h"

// Qt includes
#include <QListWidget>

// Local classes
class SharedObjects;
class Settings;
class MapWidget;

// Qt classes
class QMenu;
class QAction;
class QListWidgetItem;

class BookmarksList : public QListWidget
{
    Q_OBJECT

public:
    explicit BookmarksList(SharedObjects *sharedObjects, QWidget *parent = nullptr);

signals:
    void showInfo(const QString &id);

private: // Structs
    struct EnteredString
    {
        QString string;
        bool okay;
    };

private slots:
    void showContextMenu(const QPoint &point);
    void newBookmark();
    void itemHighlighted(QListWidgetItem *item, QListWidgetItem *);
    void renameBookmark();
    void deleteBookmark();
    void elevationProcessed(ElevationEngine::Target target, const QString &id, double elevation);
    void restoreAfterElevationLookup();


private: // Functions
    EnteredString getString(const QString &title, const QString &label,
                            const QString &text = QString());

private: // Variables
    Settings *m_settings;
    ElevationEngine *m_elevationEngine;
    MapWidget *m_mapWidget;

    QListWidgetItem *m_contextMenuItem = nullptr;
    QMenu *m_contextMenu;
    QAction *m_renameBookmark;
    QAction *m_deleteBookmark;

};

#endif // BOOKMARKSLIST_H
