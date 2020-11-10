/* SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IMAGESLISTVIEW_H
#define IMAGESLISTVIEW_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// Qt includes
#include <QListView>

// Local classes
class SharedObjects;
class ImagesListFilter;

// Qt classes
class QMenu;
class QAction;

class ImagesListView : public QListView
{
    Q_OBJECT

public:
    explicit ImagesListView(KGeoTag::ImagesListType type, SharedObjects *sharedObjects,
                            QWidget *parent = nullptr);
    void setListType(KGeoTag::ImagesListType type);
    QVector<QString> selectedPaths() const;

public slots:
    void updateBookmarks();

signals:
    void imageSelected(const QModelIndex &index) const;
    void centerImage(const QModelIndex &index) const;
    void requestAutomaticMatching(ImagesListView *list, KGeoTag::SearchType searchType) const;
    void assignToMapCenter(ImagesListView *list) const;
    void assignManually(ImagesListView *list) const;
    void editCoordinates(ImagesListView *list) const;
    void lookupElevation(ImagesListView *list) const;
    void removeCoordinates(ImagesListView *list) const;
    void removeCoordinates(const QVector<QString> &paths) const;
    void discardChanges(ImagesListView *list) const;
    void assignTo(const QVector<QString> &paths, const Coordinates &coordinates) const;
    void requestAddingImages(const QVector<QString> &paths) const;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void currentChanged(const QModelIndex &current, const QModelIndex &) override;

private slots:
    void showContextMenu(const QPoint &point);
    void selectImages(bool coordinatesSet);

private: // Variables
    KGeoTag::ImagesListType m_listType;
    ImagesListFilter *m_listFilter;
    const QHash<QString, Coordinates> *m_bookmarks;

    bool m_dragStarted = false;
    QPoint m_dragStartPosition;

    QMenu *m_contextMenu;
    QMenu *m_automaticMatchingMenu;
    QMenu *m_bookmarksMenu;
    QMenu *m_selectMenu;

    QAction *m_selectAll;
    QAction *m_assignToMapCenter;
    QAction *m_assignManually;
    QAction *m_editCoordinates;
    QAction *m_lookupElevation;
    QAction *m_removeCoordinates;
    QAction *m_discardChanges;

};

#endif // IMAGESLISTVIEW_H
