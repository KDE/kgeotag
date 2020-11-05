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

// Local includes
#include "ImagesViewFilter.h"
#include "Coordinates.h"

// Qt includes
#include <QDebug>
#include <QMimeData>
#include <QUrl>

ImagesViewFilter::ImagesViewFilter(QObject *parent, KGeoTag::ImagesListType type)
    : QSortFilterProxyModel(parent),
      m_type(type)
{
}

bool ImagesViewFilter::filterAcceptsRow(int sourceRow, const QModelIndex &) const
{
    if (m_type == KGeoTag::AllImages) {
        return true;
    }

    const bool coordinatesSet = sourceModel()->index(sourceRow, 0).data(
                                    KGeoTag::CoordinatesRole).value<Coordinates>().isSet();

    return    (m_type == KGeoTag::AssignedImages   &&   coordinatesSet)
           || (m_type == KGeoTag::UnAssignedImages && ! coordinatesSet);
}

Qt::DropActions ImagesViewFilter::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::ItemFlags ImagesViewFilter::flags(const QModelIndex &index) const
{
    auto defaultFlags = QSortFilterProxyModel::flags(index);

    if (index.isValid()) {
        return defaultFlags;
    } else {
        return Qt::ItemIsDropEnabled | defaultFlags;
    }
}

bool ImagesViewFilter::canDropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                                       const QModelIndex &) const
{
    return (action & (Qt::CopyAction | Qt::MoveAction)) && data->hasUrls();
}

bool ImagesViewFilter::dropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                                    const QModelIndex &)
{
    if (! (action & (Qt::CopyAction | Qt::MoveAction))) {
        return false;
    }

    if (data->hasUrls()) {
        qDebug() << "Accepting drop data:";
        for (const auto &url : data->urls()) {
            if (url.isLocalFile())
                qDebug() << url << "dropped";
            else
                qWarning() << url << "is not a local file! Ignoring...";
        }
    }

    return true;
}
