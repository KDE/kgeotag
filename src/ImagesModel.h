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

#ifndef IMAGESMODEL_H
#define IMAGESMODEL_H

// Qt includes
#include <QAbstractListModel>
#include <QDateTime>

// Local classes
class ImageCache;

class ImagesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ImagesModel(ImageCache *imageCache, QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex &) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool setData(const QModelIndex &index, const QVariant &value,
                         int role = Qt::DisplayRole) override;
    bool addImage(const QString &path);

private: // Variables
    ImageCache *m_imageCache;
    QVector<QString> m_paths;
    QVector<QString> m_fileNames;

};

#endif // IMAGESMODEL_H
