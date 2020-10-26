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
#include "CoordinatesFormatter.h"
#include "KGeoTag.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QLocale>

// C++ includes
#include <cmath>

CoordinatesFormatter::CoordinatesFormatter(QObject *parent, QLocale *locale)
    : QObject(parent),
      m_locale(locale)
{
}

QString CoordinatesFormatter::formatLonLat(double value) const
{
    return m_locale->toString(std::abs(value), 'f', 5);
}

QString CoordinatesFormatter::lon(const KGeoTag::Coordinates &coordinates) const
{
    return i18nc("Formatted longitude with a cardinal direction", "%1° %2",
                 formatLonLat(coordinates.lon),
                 coordinates.lon >= 0 ? i18nc("Cardinal direction", "E")
                                      : i18nc("Cardinal direction", "W"));
}

QString CoordinatesFormatter::lat(const KGeoTag::Coordinates &coordinates) const
{
    return i18nc("Formatted latitude with a cardinal direction", "%1° %2",
                 formatLonLat(coordinates.lat),
                 coordinates.lat >= 0 ? i18nc("Cardinal direction", "N")
                                      : i18nc("Cardinal direction", "S"));
}

QString CoordinatesFormatter::alt(const KGeoTag::Coordinates &coordinates) const
{
    return m_locale->toString(coordinates.alt, 'f', 1);
}
