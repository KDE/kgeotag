/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "GeoDataModel.h"

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
    return {};
}

void GeoDataModel::addTrack(const QString &path, const QVector<QVector<QDateTime>> &times,
                            const QVector<QVector<Coordinates>> &segments)
{
    const QFileInfo info(path);
    m_loadedFiles.append(info.canonicalFilePath());

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
}

bool GeoDataModel::contains(const QString &path)
{
    const QFileInfo info(path);
    return m_loadedFiles.contains(info.canonicalFilePath());
}
