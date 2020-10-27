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
    const QHash<QString, KGeoTag::Coordinates> *bookmarks() const;

signals:
    void showInfo(const KGeoTag::Coordinates &coordinates) const;
    void bookmarksChanged() const;

private: // Structs
    struct EnteredString
    {
        QString string;
        bool okay;
    };

private slots:
    void showContextMenu(const QPoint &point);
    void newBookmark();
    void newManualBookmark();
    void renameBookmark();
    void editCoordinates();
    void lookupElevation();
    void setElevation();
    void deleteBookmark();

    void itemHighlighted(QListWidgetItem *item, QListWidgetItem *);
    void elevationProcessed(ElevationEngine::Target target, const QVector<QString> &ids,
                            const QVector<double> &elevations);
    void restoreAfterElevationLookup();
    void centerBookmark(QListWidgetItem *item);


private: // Functions
    void saveBookmark(QString label, const KGeoTag::Coordinates &coordinates);
    EnteredString getString(const QString &title, const QString &label,
                            const QString &text = QString());
    void requestElevation(const QString &id);

private: // Variables
    Settings *m_settings;
    ElevationEngine *m_elevationEngine;
    MapWidget *m_mapWidget;

    QHash<QString, KGeoTag::Coordinates> m_bookmarks;

    QListWidgetItem *m_contextMenuItem = nullptr;
    QMenu *m_contextMenu;
    QAction *m_renameBookmark;
    QAction *m_lookupElevation;
    QAction *m_setElevation;
    QAction *m_deleteBookmark;
    QAction *m_editCoordinates;

};

#endif // BOOKMARKSLIST_H
