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

#ifndef KGEOTAG_H
#define KGEOTAG_H

// Qt includes
#include <QString>

namespace KGeoTag
{

struct Coordinates
{
    double lon;
    double lat;
    double alt;

    bool isSet;

    bool operator ==(const Coordinates &other) const
    {
        return    this->lon == other.lon
               && this->lat == other.lat
               && this->alt == other.alt
               && this->isSet == other.isSet;
    }

    bool operator !=(const Coordinates &other) const
    {
        return    this->lon != other.lon
               || this->lat != other.lat
               || this->alt != other.alt
               || this->isSet != other.isSet;
    }
};

const Coordinates NoCoordinates { 0.0, 0.0, 0.0, false };

enum MatchType {
    None,
    Exact,
    Interpolated,
    Set
};

QString formatLon(const Coordinates &coordinates);
QString formatLat(const Coordinates &coordinates);

}

#endif // KGEOTAG_H
