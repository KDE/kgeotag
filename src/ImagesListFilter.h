// SPDX-FileCopyrightText: 2020-2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef IMAGESLISTFILTER_H
#define IMAGESLISTFILTER_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QSortFilterProxyModel>

// Local classes
class ImagesModel;

class ImagesListFilter : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ImagesListFilter(QObject *parent, KGeoTag::ImagesListType type);
    void setSourceModel(QAbstractItemModel *sourceModel) override;
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                         const QModelIndex &) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                      const QModelIndex &) override;
    void setListType(KGeoTag::ImagesListType type);

Q_SIGNALS:
    void requestAddingImages(const QList<QString> &paths);
    void requestRemoveCoordinates(const QList<QString> &paths);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &) const override;

private: // Variables
    KGeoTag::ImagesListType m_listType;
    ImagesModel *m_imagesModel;

};

#endif // IMAGESLISTFILTER_H
