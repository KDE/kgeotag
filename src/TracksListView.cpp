// SPDX-FileCopyrightText: 2021 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "TracksListView.h"
#include "GeoDataModel.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QMenu>
#include <QAction>

TracksListView::TracksListView(GeoDataModel *model, QWidget *parent) : QListView(parent)
{
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DropOnly);

    setModel(model);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &QAbstractItemView::clicked, this, &TracksListView::trackSelected);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &TracksListView::checkSelection);

    m_contextMenu = new QMenu(this);

    m_remove = m_contextMenu->addAction(i18np("Remove track", "Remove tracks", 1));
    connect(m_remove, &QAction::triggered, this, &TracksListView::removeTracks);

    connect(this, &QListView::customContextMenuRequested, this, &TracksListView::showContextMenu);
}

void TracksListView::currentChanged(const QModelIndex &current, const QModelIndex &)
{
    if (current.isValid()) {
        emit trackSelected(current);
        scrollTo(current);
    }
}

void TracksListView::showContextMenu(const QPoint &point)
{
    const auto selected = selectedIndexes();
    const int allSelected = selected.count();

    m_remove->setEnabled(allSelected > 0);
    m_remove->setText(i18np("Remove track", "Remove tracks", allSelected));

    m_contextMenu->exec(mapToGlobal(point));
}

QVector<int> TracksListView::selectedTracks() const
{
    QVector<int> selection;
    const auto selected = selectedIndexes();
    for (const auto &index : selected) {
        selection.append(index.row());
    }
    return selection;
}

void TracksListView::checkSelection()
{
    const auto selected = selectedIndexes();
    emit updateTrackWalker(selected.count() == 1 ? selected.first().row() : -1);
}
