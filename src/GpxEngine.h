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

#ifndef GPXENGINE_H
#define GPXENGINE_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// Qt includes
#include <QObject>
#include <QVector>
#include <QHash>
#include <QDateTime>

// Local classes
class Settings;

class GpxEngine : public QObject
{
    Q_OBJECT

public:
    enum LoadResult {
        Okay,
        OpenFailed,
        XmlError
    };

    struct LoadInfo
    {
        LoadResult result;
        int points = 0;
        int segments = 0;
    };

    explicit GpxEngine(QObject *parent, Settings *settings);
    GpxEngine::LoadInfo load(const QString &path);
    Coordinates findExactCoordinates(const QDateTime &time, int deviation) const;
    Coordinates findInterpolatedCoordinates(const QDateTime &time, int deviation) const;

signals:
    void segmentLoaded(const QVector<Coordinates> &segment) const;

private: // Functions
    Coordinates findExactCoordinates(const QDateTime &time) const;
    Coordinates findInterpolatedCoordinates(const QDateTime &time) const;

private: // Variables
    Settings *m_settings;
    QVector<QDateTime> m_allTimes;
    QHash<QDateTime, Coordinates> m_coordinates;

};

#endif // GPXENGINE_H
