// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "CoordinatesFormatter.h"
#include "KGeoTag.h"
#include "Coordinates.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QLocale>

// C++ includes
#include <cmath>

CoordinatesFormatter::CoordinatesFormatter(QObject *parent, QLocale *locale,
                                           const bool *latBeforeLon)
    : QObject(parent),
      m_locale(locale),
      m_latBeforeLon(latBeforeLon)
{
}

QString CoordinatesFormatter::formatLonLat(double value) const
{
    return m_locale->toString(std::abs(value), 'f', KGeoTag::degreesPrecision);
}

QString CoordinatesFormatter::format(const Coordinates &coordinates) const
{
    return i18nc("Formatted coordinates, \"lon, lat\" or \"lat, lon\"", "%1, %2",
                 *m_latBeforeLon ? lat(coordinates) : lon(coordinates),
                 *m_latBeforeLon ? lon(coordinates) : lat(coordinates));
}

QString CoordinatesFormatter::lon(const Coordinates &coordinates) const
{
    return i18nc("Formatted longitude with a cardinal direction", "%1° %2",
                 formatLonLat(coordinates.lon()),
                 coordinates.lon() >= 0 ? i18nc("Abbreviated cardinal direction \"East\"", "E")
                                        : i18nc("Abbreviated cardinal direction \"West\"", "W"));
}

QString CoordinatesFormatter::lat(const Coordinates &coordinates) const
{
    return i18nc("Formatted latitude with a cardinal direction", "%1° %2",
                 formatLonLat(coordinates.lat()),
                 coordinates.lat() >= 0 ? i18nc("Abbreviated cardinal direction \"North\"", "N")
                                        : i18nc("Abbreviated cardinal direction \"South\"", "S"));
}

QString CoordinatesFormatter::alt(const Coordinates &coordinates) const
{
    return i18nc("Formatted altitude in meters", "%1 m",
                 m_locale->toString(coordinates.alt(), 'f', KGeoTag::altitudePrecision));
}
