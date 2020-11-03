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
#include "ImagesModel.h"
#include "ImageCache.h"

// Qt includes
#include <QDebug>

ImagesViewFilter::ImagesViewFilter(QObject *parent, KGeoTag::ImagesListType type,
                                   ImageCache *imageCache)
    : QSortFilterProxyModel(parent),
      m_type(type),
      m_imageCache(imageCache)
{
}

bool ImagesViewFilter::filterAcceptsRow(int sourceRow, const QModelIndex &) const
{
    if (m_type == KGeoTag::ImagesListType::All) {
        return true;
    }

    const auto path = sourceModel()->index(sourceRow, 0).data(
                          ImagesModel::DataRole::Path).toString();

    if (m_type == KGeoTag::ImagesListType::Assigned) {
        return m_imageCache->coordinates(path) != KGeoTag::NoCoordinates;
    } else {
        return m_imageCache->coordinates(path) == KGeoTag::NoCoordinates;
    }
}
