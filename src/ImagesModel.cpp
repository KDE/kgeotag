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
#include "KGeoTag.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QFileInfo>
#include <QFont>

// C++ includes
#include <utility>

ImagesModel::ImagesModel(QObject *parent, ImageCache *imageCache)
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

    const auto &path = m_paths.at(index.row());
    const auto &data = m_imageData[path];

    if (role == Qt::DisplayRole) {
        return i18nc("Pattern for a display filename with a \"changed\" and a \"associated\" "
                     "marker",
                     "%1%2%3",
                     m_imageCache->coordinates(path) != KGeoTag::NoCoordinates
                        ? i18nc("Marker for an associated file", "\u2713\u2009")
                        : QString(),
                     data.fileName,
                     m_imageData.value(path).changed
                        ? i18nc("Marker for a changed file", "\u2009*")
                        : QString());

    } else if (role == Qt::DecorationRole) {
        return m_imageCache->thumbnail(path);

    } else if (role == Qt::ForegroundRole) {
        switch (m_imageData.value(path).matchType) {
        case KGeoTag::MatchType::None:
            return m_colorScheme.foreground();
        case KGeoTag::MatchType::Exact:
            return m_colorScheme.foreground(KColorScheme::PositiveText);
        case KGeoTag::MatchType::Interpolated:
            return m_colorScheme.foreground(KColorScheme::NeutralText);
        case KGeoTag::MatchType::Set:
            return m_colorScheme.foreground(KColorScheme::LinkText);
        }

    } else if (role == Qt::FontRole) {
        QFont font;
        if (m_imageCache->coordinates(path) != KGeoTag::NoCoordinates) {
            font.setBold(true);
        }
        return font;

    } else if (role == DataRole::Path) {
        return path;

    } else if (role == DataRole::Changed) {
        return m_imageData.value(path).changed;

    }

    return QVariant();
}

void ImagesModel::addImage(const QString &path)
{
    // Check if we already have the image
    if (m_paths.contains(path)) {
        return;
    }

    // Find the correct row for the new image (sorted by date)
    const auto newDate = m_imageCache->date(path);
    int row = 0;
    for (const QString &cachePath : m_paths) {
        if (m_imageCache->date(cachePath) > newDate) {
            break;
        }
        row++;
    }

    // Add the image

    beginInsertRows(QModelIndex(), row, row);
    m_paths.insert(row, path);
    const QFileInfo info(path);
    m_imageData[path] = { info.fileName(),
                          false,
                          KGeoTag::MatchType::None };
    endInsertRows();

    const auto modelIndex = index(row, 0, QModelIndex());
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });
}

void ImagesModel::emitDataChanged(const QString &path)
{
    const auto modelIndex = index(m_paths.indexOf(path), 0, QModelIndex());
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });
}

void ImagesModel::setChanged(const QString &path, bool changed)
{
    m_imageData[path].changed = changed;
    emitDataChanged(path);
}

void ImagesModel::setMatchType(const QString &path, int matchType)
{
    m_imageData[path].matchType = matchType;
    emitDataChanged(path);
}

QVector<QString> ImagesModel::changedImages() const
{
    QVector<QString> changed;
    for (const auto &path : std::as_const(m_paths)) {
        if (m_imageData.value(path).changed) {
            changed.append(path);
        }
    }
    return changed;
}
