// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

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
