// SPDX-FileCopyrightText: 2020-2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
class CoordinatesParser;

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
    QList<QString> selectedPaths() const;
    void highlightImage(const QModelIndex &index);
    void selectFirstUnassigned();

public Q_SLOTS:
    void updateBookmarks();

Q_SIGNALS:
    void imageSelected(const QModelIndex &index);
    void centerImage(const QModelIndex &index);
    void requestAutomaticMatching(ImagesListView *list, KGeoTag::SearchType searchType);
    void assignToMapCenter(ImagesListView *list);
    void assignManually(ImagesListView *list);
    void editCoordinates(ImagesListView *list);
    void lookupElevation(ImagesListView *list);
    void removeCoordinates(ImagesListView *list);
    void removeCoordinates(const QList<QString> &paths);
    void requestSaving(ImagesListView *list);
    void discardChanges(ImagesListView *list);
    void assignTo(const QList<QString> &paths, const Coordinates &coordinates);
    void requestAddingImages(const QList<QString> &paths);
    void removeImages(ImagesListView *list);
    void failedToParseClipboard();
    void findClosestTrackPoint(const QString &path);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void currentChanged(const QModelIndex &current, const QModelIndex &) override;

private Q_SLOTS:
    void showContextMenu(const QPoint &point);
    void selectImages(bool coordinatesSet);
    void openExternally();
    void assignToClipboard();

private: // Variables
    KGeoTag::ImagesListType m_listType;
    ImagesListFilter *m_listFilter;
    const QHash<QString, Coordinates> *m_bookmarks;
    CoordinatesParser *m_coordinatesParser;

    bool m_dragStarted = false;
    QPoint m_dragStartPosition;

    QMenu *m_contextMenu;
    QMenu *m_automaticMatchingMenu;
    QMenu *m_bookmarksMenu;
    QMenu *m_selectMenu;

    QAction *m_selectAll;
    QAction *m_assignToMapCenter;
    QAction *m_assignToClipboard;
    QAction *m_assignManually;
    QAction *m_findClosestTrackPoint;
    QAction *m_editCoordinates;
    QAction *m_lookupElevation;
    QAction *m_removeCoordinates;
    QAction *m_save;
    QAction *m_discardChanges;
    QAction *m_removeImages;
    QAction *m_openExternally;

};

#endif // IMAGESLISTVIEW_H
