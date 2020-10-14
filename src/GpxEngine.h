/* Copyright (c) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef GPXENGINE_H
#define GPXENGINE_H

// Local includes
#include "KGeoTag.h"

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

    explicit GpxEngine(QObject *parent, Settings *settings);
    GpxEngine::LoadResult load(const QString &path);
    KGeoTag::Coordinates findExactCoordinates(const QDateTime &time, int deviation) const;
    KGeoTag::Coordinates findInterpolatedCoordinates(const QDateTime &time, int deviation) const;

signals:
    void segmentLoaded(const QVector<KGeoTag::Coordinates> &segment) const;

private: // Functions
    KGeoTag::Coordinates findExactCoordinates(const QDateTime &time) const;
    KGeoTag::Coordinates findInterpolatedCoordinates(const QDateTime &time) const;

private: // Variables
    Settings *m_settings;
    QVector<QDateTime> m_allTimes;
    QHash<QDateTime, KGeoTag::Coordinates> m_coordinates;

};

#endif // GPXENGINE_H
