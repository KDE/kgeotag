// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

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

QDebug operator<<(QDebug debug, const Coordinates &coordinates)
{
    QDebugStateSaver saver(debug);

    if (! coordinates.isSet()) {
        debug.nospace() << "Coordinates(not set)";
    } else {
        debug.nospace() << "Coordinates("
                        <<   "lon: " << coordinates.lon()
                        << ", lat: " << coordinates.lat()
                        << ", alt: " << coordinates.alt()
                        << ')';
    }

    return debug;
}
