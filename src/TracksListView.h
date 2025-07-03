// SPDX-FileCopyrightText: 2021-2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef TRACKSLISTVIEW_H
#define TRACKSLISTVIEW_H

// Qt includes
#include <QListView>

// Local classes
class GeoDataModel;

// Qt classes
class QMenu;
class QAction;

class TracksListView : public QListView
{
    Q_OBJECT

public:
    explicit TracksListView(GeoDataModel *model, QWidget *parent = nullptr);
    QList<int> selectedTracks() const;

Q_SIGNALS:
    void trackSelected(const QModelIndex &index);
    void removeTracks();
    void updateTrackWalker(int row);

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &) override;

private Q_SLOTS:
    void showContextMenu(const QPoint &point);
    void checkSelection();

private: // Variables
    QMenu *m_contextMenu;
    QAction *m_remove;

};

#endif // TRACKSLISTVIEW_H
