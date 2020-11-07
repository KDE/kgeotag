/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "ImagesViewFilter.h"
#include "Coordinates.h"
#include "ImagesModel.h"

// Qt includes
#include <QDebug>
#include <QMimeData>
#include <QUrl>

ImagesViewFilter::ImagesViewFilter(QObject *parent, KGeoTag::ImagesListType type)
    : QSortFilterProxyModel(parent),
      m_type(type)
{
}

void ImagesViewFilter::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
    m_imagesModel = qobject_cast<ImagesModel *>(sourceModel);
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
    if (! (action & (Qt::CopyAction | Qt::MoveAction)) || ! data->hasUrls()) {
        return false;
    }

    // Check if the drag's origin is an image list
    const auto source = data->data(KGeoTag::SourceImagesListMimeType);
    if (! source.isEmpty()) {
        // Don't allow drops on the "assigned" images list
        if (m_type == KGeoTag::AssignedImages) {
            return false;
        }
        // Don't allow drops on the origin list
        if (source == KGeoTag::SourceImagesList.value(m_type)) {
            return false;
        }
    }

    return true;
}

bool ImagesViewFilter::dropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                                    const QModelIndex &)
{
    if (! (action & (Qt::CopyAction | Qt::MoveAction)) || ! data->hasUrls()) {
        return false;
    }

    QVector<QString> paths;
    QVector<QString> removeCoordinates;

    for (const auto &url : data->urls()) {
        if (! url.isLocalFile()) {
            continue;
        }

        const auto path = url.toLocalFile();
        if (! m_imagesModel->contains(path)) {
            paths.append(path);
        } else {
            if (m_type == KGeoTag::UnAssignedImages && m_imagesModel->coordinates(path).isSet()) {
                removeCoordinates.append(path);
            }
        }
    }

    if (! removeCoordinates.isEmpty()) {
        emit requestRemoveCoordinates(removeCoordinates);
    }

    if (! paths.isEmpty()) {
        emit requestAddingImages(paths);
    }

    return true;
}
