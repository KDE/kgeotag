/* SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

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
    virtual void setSourceModel(QAbstractItemModel *sourceModel) override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                                 const QModelIndex &) const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                              const QModelIndex &) override;
    void setListType(KGeoTag::ImagesListType type);

signals:
    void requestAddingImages(const QVector<QString> &paths) const;
    void requestRemoveCoordinates(const QVector<QString> &paths) const;

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &) const override;

private: // Variables
    KGeoTag::ImagesListType m_listType;
    ImagesModel *m_imagesModel;

};

#endif // IMAGESLISTFILTER_H