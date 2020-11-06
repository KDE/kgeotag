/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef BOOKMARKSLIST_H
#define BOOKMARKSLIST_H

// Local includes
#include "ElevationEngine.h"
#include "Coordinates.h"

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
    const QHash<QString, Coordinates> *bookmarks() const;

signals:
    void showInfo(const Coordinates &coordinates) const;
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
    void saveBookmark(QString label, const Coordinates &coordinates);
    EnteredString getString(const QString &title, const QString &label,
                            const QString &text = QString());
    void requestElevation(const QString &id);

private: // Variables
    Settings *m_settings;
    ElevationEngine *m_elevationEngine;
    MapWidget *m_mapWidget;

    QHash<QString, Coordinates> m_bookmarks;

    QListWidgetItem *m_contextMenuItem = nullptr;
    QMenu *m_contextMenu;
    QAction *m_renameBookmark;
    QAction *m_lookupElevation;
    QAction *m_setElevation;
    QAction *m_deleteBookmark;
    QAction *m_editCoordinates;

};

#endif // BOOKMARKSLIST_H
