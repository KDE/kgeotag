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

#ifndef IMAGESVIEWFILTER_H
#define IMAGESVIEWFILTER_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QSortFilterProxyModel>

// Local classes
class SharedObjects;
class ImagesModel;

class ImagesViewFilter : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ImagesViewFilter(QObject *parent, KGeoTag::ImagesListType type,
                              SharedObjects *sharedObjects);

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &) const override;

private: // Variables
    KGeoTag::ImagesListType m_type;
    ImagesModel *m_imagesModel;

};

#endif // IMAGESVIEWFILTER_H
