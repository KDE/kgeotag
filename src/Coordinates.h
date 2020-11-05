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

#ifndef COORDINATES_H
#define COORDINATES_H

// Qt includes
#include <QMetaType>
#include <QDebug>

class Coordinates
{

public:
    explicit Coordinates();
    explicit Coordinates(double lon, double lat, double alt, bool isSet);
    double lon() const;
    double lat() const;
    void setAlt(double alt);
    double alt() const;
    bool isSet() const;
    bool operator==(const Coordinates &other) const;
    bool operator!=(const Coordinates &other) const;

private: // Variables
    double m_lon = 0.0;
    double m_lat = 0.0;
    double m_alt = 0.0;
    bool m_isSet = false;

};

QDebug operator<<(QDebug debug, const Coordinates &coordinates);

Q_DECLARE_METATYPE(Coordinates)

#endif // COORDINATES_H