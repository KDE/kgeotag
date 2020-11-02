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
#include "ImagesModel.h"
#include "ImageCache.h"

// Qt includes
#include <QFileInfo>

ImagesModel::ImagesModel(ImageCache *imageCache, QObject *parent)
    : QAbstractListModel(parent),
      m_imageCache(imageCache)
{
}

int ImagesModel::rowCount(const QModelIndex &) const
{
    return m_paths.count();
}

QVariant ImagesModel::data(const QModelIndex &index, int role) const
{
    if (! index.isValid() || index.row() > m_paths.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole){
        const int row = index.row();
        return m_imageCache->changed(m_paths.at(row))
            ? QStringLiteral("*") + m_fileNames.at(row)
            : m_fileNames.at(row);

    } else if (role == Qt::DecorationRole) {
        return m_imageCache->thumbnail(m_paths.at(index.row()));
    }

    return QVariant();
}

bool ImagesModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1 || row < 0 || row > rowCount(parent)) {
        return false;
    }

    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; r++) {
        m_paths.insert(row, QString());
        m_fileNames.insert(row, QString());
    }

    endInsertRows();

    return true;
}

bool ImagesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= 0 && index.row() < m_paths.size()
        && (role == Qt::EditRole || role == Qt::DisplayRole)) {

        const auto path = value.toString();

        if (m_paths.at(index.row()) == path) {
            return true;
        }

        m_paths.replace(index.row(), path);

        const QFileInfo info(path);
        m_fileNames.replace(index.row(), info.fileName());

        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
        return true;
    }

    return false;
}

bool ImagesModel::addImage(const QString &path)
{
    if (m_paths.contains(path)) {
        return true;
    }

    const int row = m_paths.count();
    if (insertRows(row, 1)) {
        return setData(index(row, 0, QModelIndex()), path);
    }
    return false;
}
