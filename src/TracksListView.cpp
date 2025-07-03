// SPDX-FileCopyrightText: 2021-2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "TracksListView.h"
#include "GeoDataModel.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QMenu>
#include <QAction>

// C++ includes
#include <functional>

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
    connect(m_remove, &QAction::triggered,
            this, std::bind(&TracksListView::updateTrackWalker, this, -1));

    connect(this, &QListView::customContextMenuRequested, this, &TracksListView::showContextMenu);
}

void TracksListView::currentChanged(const QModelIndex &current, const QModelIndex &)
{
    if (current.isValid()) {
        Q_EMIT trackSelected(current);
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

QList<int> TracksListView::selectedTracks() const
{
    QList<int> selection;
    const auto selected = selectedIndexes();
    for (const auto &index : selected) {
        selection.append(index.row());
    }
    return selection;
}

void TracksListView::checkSelection()
{
    const auto selected = selectedIndexes();
    Q_EMIT updateTrackWalker(selected.count() == 1 ? selected.first().row() : -1);
}
