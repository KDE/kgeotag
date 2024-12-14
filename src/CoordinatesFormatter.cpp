// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "CoordinatesFormatter.h"
#include "KGeoTag.h"
#include "Coordinates.h"
#include "Settings.h"
#include "DegreesConverter.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QLocale>

// C++ includes
#include <cmath>

CoordinatesFormatter::CoordinatesFormatter(QObject *parent, QLocale *locale, Settings *settings)
    : QObject(parent),
      m_locale(locale),
      m_settings(settings)
{
}

QString CoordinatesFormatter::formatLonLat(double value) const
{
    switch (m_settings->coordinatesFlavor()) {

    case KGeoTag::DecimalDegrees:
        return i18nc("Formatted coordinates as decimal degrees", "%1°",
                     m_locale->toString(std::abs(value), 'f', KGeoTag::degreesPrecision));

    case KGeoTag::DegreesDecimalMinutes:
    {
        double decimalMinutes;
        const auto degrees = DegreesConverter::fromDecimal(std::abs(value), &decimalMinutes);
        return i18nc("Formatted coordinates as degrees and decimal minutes", "%1° %2'",
                     QString::number(degrees),
                     m_locale->toString(decimalMinutes, 'f', KGeoTag::minutesPrecision));
    }

    case KGeoTag::DegreesMinutesDecimalSeconds:
    {
        double minutes;
        double decimalSeconds;
        const auto degrees = DegreesConverter::fromDecimal(std::abs(value),
                                                           &minutes, &decimalSeconds);
        return i18nc("Formatted coordinates as degrees, minutes and decimal seconds",
                     "%1° %2' %3\"",
                     QString::number(degrees),
                     QString::number(minutes),
                     m_locale->toString(decimalSeconds, 'f', KGeoTag::secondsPrecision));
    }

    }

    // We can't reach here, but the compiler thinks so
    return QString();
}

QString CoordinatesFormatter::format(const Coordinates &coordinates) const
{
    return i18nc("Formatted coordinates, \"lon, lat\" or \"lat, lon\"", "%1, %2",
                 m_settings->latBeforeLon() ? lat(coordinates) : lon(coordinates),
                 m_settings->latBeforeLon() ? lon(coordinates) : lat(coordinates));
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

QString CoordinatesFormatter::formatDecLatLon(const Coordinates &coordinates) const
{
    return QStringLiteral("%1, %2").arg(
        QStringLiteral("%1").arg(coordinates.lat(), 0, 'f', KGeoTag::degreesPrecision),
        QStringLiteral("%1").arg(coordinates.lon(), 0, 'f', KGeoTag::degreesPrecision));
}
