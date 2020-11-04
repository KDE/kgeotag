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

// Local includes
#include "Coordinates.h"

Coordinates::Coordinates()
{
}

Coordinates::Coordinates(double lon, double lat, double alt, bool isSet)
    : m_lon(lon),
      m_lat(lat),
      m_alt(alt),
      m_isSet(isSet)
{
}

double Coordinates::lon() const
{
    return m_lon;
}

double Coordinates::lat() const
{
    return m_lat;
}

void Coordinates::setAlt(double alt)
{
    m_alt = alt;
}

double Coordinates::alt() const
{
    return m_alt;
}

bool Coordinates::isSet() const
{
    return m_isSet;
}

bool Coordinates::operator==(const Coordinates &other) const
{
    return     m_lon == other.lon()
            && m_lat == other.lat()
            && m_alt == other.alt()
            && m_isSet == other.isSet();
}

bool Coordinates::operator!=(const Coordinates &other) const
{
    return     m_lon != other.lon()
            || m_lat != other.lat()
            || m_alt != other.alt()
            || m_isSet != other.isSet();
}
