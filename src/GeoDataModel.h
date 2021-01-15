/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GEODATAMODEL_H
#define GEODATAMODEL_H

// Qt includes
#include <QAbstractListModel>

class GeoDataModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GeoDataModel(QObject *parent);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addTrack(const QString &path);
    bool contains(const QString &path);

private: // Variables
    QVector<QString> m_loadedFiles;

};

#endif // GEODATAMODEL_H
