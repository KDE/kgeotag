/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "GeoDataModel.h"

GeoDataModel::GeoDataModel(QObject *parent) : QAbstractListModel(parent)
{
}

int GeoDataModel::rowCount(const QModelIndex &) const
{
    return 0;
}

QVariant GeoDataModel::data(const QModelIndex &index, int role) const
{
    return {};
}
