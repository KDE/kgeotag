/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "GeoDataModel.h"

// Qt includes
#include <QFileInfo>

GeoDataModel::GeoDataModel(QObject *parent) : QAbstractListModel(parent)
{
}

int GeoDataModel::rowCount(const QModelIndex &) const
{
    return m_loadedFiles.count();
}

QVariant GeoDataModel::data(const QModelIndex &index, int role) const
{
    return {};
}

void GeoDataModel::addTrack(const QString &path)
{
    const QFileInfo info(path);
    m_loadedFiles.append(info.canonicalFilePath());
}

bool GeoDataModel::contains(const QString &path)
{
    const QFileInfo info(path);
    return m_loadedFiles.contains(info.canonicalFilePath());
}
