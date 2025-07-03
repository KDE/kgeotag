// SPDX-FileCopyrightText: 2021-2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef GEODATAMODEL_H
#define GEODATAMODEL_H

// Local includes
#include "Coordinates.h"

// Marble includes
#include <marble/GeoDataLineString.h>
#include <marble/GeoDataLatLonAltBox.h>

// Qt includes
#include <QAbstractListModel>
#include <QHash>
#include <QDateTime>

class GeoDataModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GeoDataModel(QObject *parent);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                         const QModelIndex &) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int, int,
                      const QModelIndex &) override;

    bool contains(const QString &path);
    void addTrack(const QString &path, const QList<QList<QDateTime>> &times,
                  const QList<QList<Coordinates>> &segments);
    void removeTrack(int row);
    void removeAllTracks();
    Marble::GeoDataLatLonAltBox trackBox(const QString &path) const;
    Marble::GeoDataLatLonAltBox trackBox(const QModelIndex &index) const;
    Coordinates trackBoxCenter(const QString &path) const;

    const QList<QList<Marble::GeoDataLineString>> &marbleTracks() const;
    const QList<QList<QDateTime>> &dateTimes() const;
    const QList<QHash<QDateTime, Coordinates>> &trackPoints() const;

Q_SIGNALS:
    void requestAddFiles(const QList<QString> &paths);

private: // Functions
    QString canonicalPath(const QString &path) const;

private: // Variables
    QList<QString> m_loadedFiles;
    QList<QString> m_displayFileNames;

    QList<QList<Marble::GeoDataLineString>> m_marbleTracks;
    QList<Marble::GeoDataLatLonAltBox> m_marbleTrackBoxes;

    QList<QList<QDateTime>> m_dateTimes;
    QList<QHash<QDateTime, Coordinates>> m_trackPoints;

};

#endif // GEODATAMODEL_H
