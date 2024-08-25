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
    // Decimal degrees
        return i18nc("Formatted coordinates as decimal degrees", "%1°",
                     m_locale->toString(std::abs(value), 'f', KGeoTag::degreesPrecision));

    /*
    // Degrees, decial minutes
        double decimals;
        double degrees;
        double decimalMinutes;
        decimals = std::modf(value, &degrees);
        decimalMinutes = decimals * 60.0;
        return i18nc("Formatted coordinates as degrees and decimal minutes", "%1° %2'",
                     QString::number(std::abs(degrees)),
                     m_locale->toString(std::abs(decimalMinutes), 'f', KGeoTag::minutesPrecision));

    // Degrees, minutes, decimal seconds
        double decimals;
        double degrees;
        double decimalMinutes;
        double minutes;
        double decimalSeconds;
        decimals = std::modf(value, &degrees);
        decimalMinutes = decimals * 60.0;
        decimals = std::modf(decimalMinutes, &minutes);
        decimalSeconds = decimals * 60.0;
        return i18nc("Formatted coordinates as degrees, minutes and decimal seconds",
                     "%1° %2' %3\"",
                     QString::number(std::abs(degrees)),
                     QString::number(std::abs(minutes)),
                     m_locale->toString(std::abs(decimalSeconds), 'f', KGeoTag::secondsPrecision));
    */
}

QString CoordinatesFormatter::format(const Coordinates &coordinates) const
{
    return i18nc("Formatted coordinates, \"lon, lat\" or \"lat, lon\"", "%1, %2",
                 *m_latBeforeLon ? lat(coordinates) : lon(coordinates),
                 *m_latBeforeLon ? lon(coordinates) : lat(coordinates));
}

QString CoordinatesFormatter::lon(const Coordinates &coordinates) const
{
    return i18nc("Formatted latitude or longitude with a cardinal direction", "%1 %2",
                 formatLonLat(coordinates.lon()),
                 coordinates.lon() >= 0 ? i18nc("Abbreviated cardinal direction \"East\"", "E")
                                        : i18nc("Abbreviated cardinal direction \"West\"", "W"));
}

QString CoordinatesFormatter::lat(const Coordinates &coordinates) const
{
    return i18nc("Formatted latitude or longitude with a cardinal direction", "%1 %2",
                 formatLonLat(coordinates.lat()),
                 coordinates.lat() >= 0 ? i18nc("Abbreviated cardinal direction \"North\"", "N")
                                        : i18nc("Abbreviated cardinal direction \"South\"", "S"));
}

QString CoordinatesFormatter::alt(const Coordinates &coordinates) const
{
    return i18nc("Formatted altitude in meters", "%1 m",
                 m_locale->toString(coordinates.alt(), 'f', KGeoTag::altitudePrecision));
}
