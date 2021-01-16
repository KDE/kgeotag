/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

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
    QVector<int> selectedTracks() const;

signals:
    void trackSelected(const QModelIndex &index);
    void removeTracks();
    void updateTrackWalker(int row);

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &) override;

private slots:
    void showContextMenu(const QPoint &point);
    void checkSelection();

private: // Variables
    QMenu *m_contextMenu;
    QAction *m_remove;

};

#endif // TRACKSLISTVIEW_H
