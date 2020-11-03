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

#ifndef IMAGESLISTVIEW_H
#define IMAGESLISTVIEW_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QListView>

// Local classes
class SharedObjects;

// Qt classes
class QMenu;
class QAction;

class ImagesListView : public QListView
{
    Q_OBJECT

public:
    explicit ImagesListView(KGeoTag::ImagesListType type, SharedObjects *sharedObjects,
                            QWidget *parent = nullptr);
    QVector<QString> selectedPaths() const;

public slots:
    void updateBookmarks();

signals:
    void imageSelected(const QString &path) const;
    void centerImage(const QModelIndex &index) const;

    void searchExactMatches(ImagesListView *list) const;
    void searchInterpolatedMatches(ImagesListView *list) const;
    void assignToMapCenter(ImagesListView *list) const;
    void assignManually(ImagesListView *list) const;
    void editCoordinates(ImagesListView *list) const;
    void lookupElevation(ImagesListView *list) const;
    void removeCoordinates(ImagesListView *list) const;
    void discardChanges(ImagesListView *list) const;

    void assignTo(const QVector<QString> &paths, const KGeoTag::Coordinates &coordinates) const;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private slots:
    void processImageClicked(const QModelIndex &index) const;
    void showContextMenu(const QPoint &point);

private: // Functions
    void checkCenterImage(const QModelIndex &index) const;

private: // Variables
    const QHash<QString, KGeoTag::Coordinates> *m_bookmarks;

    bool m_dragStarted = false;
    QPoint m_dragStartPosition;

    QMenu *m_contextMenu;
    QMenu *m_bookmarksMenu;
    QAction *m_assignManually;
    QAction *m_editCoordinates;
    QAction *m_lookupElevation;
    QAction *m_removeCoordinates;
    QAction *m_discardChanges;

};

#endif // IMAGESLISTVIEW_H
