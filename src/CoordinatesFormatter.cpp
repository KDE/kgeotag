// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

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

CoordinatesFormatter::CoordinatesFormatter(QObject *parent, QLocale *locale)
    : QObject(parent),
      m_locale(locale)
{
}

QString CoordinatesFormatter::formatLonLat(double value) const
{
    return m_locale->toString(std::abs(value), 'f', KGeoTag::degreesPrecision);
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
    return m_locale->toString(coordinates.alt(), 'f', KGeoTag::altitudePrecision);
}
