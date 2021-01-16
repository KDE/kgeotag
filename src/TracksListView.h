/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TRACKSLISTVIEW_H
#define TRACKSLISTVIEW_H

// Qt includes
#include <QListView>

// Local classes
class GeoDataModel;

class TracksListView : public QListView
{
    Q_OBJECT

public:
    explicit TracksListView(GeoDataModel *model, QWidget *parent = nullptr);

signals:
    void trackSelected(const QModelIndex &index);

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &) override;

};

#endif // TRACKSLISTVIEW_H
