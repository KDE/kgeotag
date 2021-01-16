/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "GeoDataModel.h"
#include "KGeoTag.h"

// Marble includes
#include <marble/GeoDataCoordinates.h>

// Qt includes
#include <QDebug>
#include <QFileInfo>

// C++ includes
#include <algorithm>

GeoDataModel::GeoDataModel(QObject *parent) : QAbstractListModel(parent)
{
}

int GeoDataModel::rowCount(const QModelIndex &) const
{
    return m_loadedFiles.count();
}

QVariant GeoDataModel::data(const QModelIndex &index, int role) const
{
    if (! index.isValid() || index.row() > m_loadedFiles.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_displayFileNames.at(index.row());
    }

    return QVariant();
}

bool GeoDataModel::contains(const QString &path)
{
    return m_loadedFiles.contains(canonicalPath(path));
}

void GeoDataModel::addTrack(const QString &path, const QVector<QVector<QDateTime>> &times,
                            const QVector<QVector<Coordinates>> &segments)
{
    Marble::GeoDataLatLonAltBox marbleTrackBox;
    QVector<Marble::GeoDataLineString> marbleTracks;

    QVector<QDateTime> dateTimes;
    QHash<QDateTime, Coordinates> trackPoints;

    for (int i = 0; i < times.count(); i++) {
        Marble::GeoDataLineString lineString;

        for (int j = 0; j < times.at(i).count(); j++) {
            const auto &coordinates = segments.at(i).at(j);
            const Marble::GeoDataCoordinates marbleCoordinates
                = Marble::GeoDataCoordinates(coordinates.lon(), coordinates.lat(), 0.0,
                                            Marble::GeoDataCoordinates::Degree);
            lineString.append(marbleCoordinates);

            const auto &dateTime = times.at(i).at(j);
            dateTimes.append(dateTime);
            trackPoints[dateTime] = coordinates;
        }

        const auto box = lineString.latLonAltBox();
        if (marbleTrackBox.isEmpty()) {
            marbleTrackBox = box;
        } else {
            marbleTrackBox |= box;
        }
        marbleTracks.append(lineString);
    }

    m_marbleTracks.append(marbleTracks);
    m_marbleTrackBoxes.append(marbleTrackBox);

    std::sort(dateTimes.begin(), dateTimes.end());
    m_dateTimes.append(dateTimes);
    m_trackPoints.append(trackPoints);

    m_loadedFiles.append(canonicalPath(path));
    const QFileInfo info(path);
    m_displayFileNames.append(info.completeBaseName());

    const auto modelIndex = index(m_displayFileNames.count() - 1, 0);
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });
}

void GeoDataModel::removeTrack(int row)
{
    beginRemoveRows(QModelIndex(), row, row);
    const auto modelIndex = index(row, 0);
    m_loadedFiles.remove(row);
    m_displayFileNames.remove(row);
    m_marbleTracks.remove(row);
    m_marbleTrackBoxes.remove(row);
    m_dateTimes.remove(row);
    m_trackPoints.remove(row);
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });
    endRemoveRows();
}

void GeoDataModel::removeAllTracks()
{
    const auto lastRow = m_loadedFiles.count() - 1;
    const auto firstModelIndex = index(0, 0);
    const auto lastModelIndex = index(lastRow, 0);
    beginRemoveRows(QModelIndex(), 0, lastRow);
    m_loadedFiles.clear();
    m_displayFileNames.clear();
    m_marbleTracks.clear();
    m_marbleTrackBoxes.clear();
    m_dateTimes.clear();
    m_trackPoints.clear();
    emit dataChanged(firstModelIndex, lastModelIndex, { Qt::DisplayRole });
    endRemoveRows();
}

QString GeoDataModel::canonicalPath(const QString &path) const
{
    const QFileInfo info(path);
    return info.canonicalFilePath();
}

Marble::GeoDataLatLonAltBox GeoDataModel::trackBox(const QString &path) const
{
    return m_marbleTrackBoxes.at(m_loadedFiles.indexOf(canonicalPath(path)));
}

Coordinates GeoDataModel::trackBoxCenter(const QString &path) const
{
    const auto center = trackBox(path).center();
    return Coordinates(center.longitude(Marble::GeoDataCoordinates::Degree),
                       center.latitude(Marble::GeoDataCoordinates::Degree),
                       0.0,
                       true);
}

Marble::GeoDataLatLonAltBox GeoDataModel::trackBox(const QModelIndex &index) const
{
    return m_marbleTrackBoxes.at(index.row());
}

const QVector<QVector<Marble::GeoDataLineString>> &GeoDataModel::marbleTracks() const
{
    return m_marbleTracks;
}

const QVector<QVector<QDateTime>> &GeoDataModel::dateTimes() const
{
    return m_dateTimes;
}

const QVector<QHash<QDateTime, Coordinates>> &GeoDataModel::trackPoints() const
{
    return m_trackPoints;
}
