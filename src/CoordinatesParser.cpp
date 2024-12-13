// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "CoordinatesParser.h"
#include "Logging.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QLocale>
#include <QRegularExpression>

static const QRegularExpression s_googleMaps(
    QStringLiteral("^(-?\\d+\\.\\d+), (-?\\d+\\.\\d+)$"));
static const QRegularExpression s_openStreetMap(
    QStringLiteral("^geo:(-?\\d+\\.\\d+),(-?\\d+\\.\\d+)\\?z=\\d+$"));

CoordinatesParser::CoordinatesParser(QObject *parent, QLocale *locale)
    : QObject(parent),
      m_locale(locale)
{
}

Coordinates CoordinatesParser::parse(const QString &input) const
{
    qCDebug(KGeoTagLog) << "Parsing coordinates string" << input;
    double lon = 0.0;
    double lat = 0.0;
    bool success = false;

    parseGoogleMaps(input, &lon, &lat, &success);
    if (success) {
        qCDebug(KGeoTagLog) << "Detected Google Maps coordinates with"
                            << "lon" << lon << "and lat" << lat;
        return Coordinates(lon, lat, 0.0, true);
    }

    parseOpenStreetMap(input, &lon, &lat, &success);
    if (success) {
        qCDebug(KGeoTagLog) << "Detected OpenStreetMap coordinates with"
                            << "lon" << lon << "and lat" << lat;
        return Coordinates(lon, lat, 0.0, true);
    }

    qCDebug(KGeoTagLog) << "Could not extract coordinates from" << input;
    return Coordinates();
}

void CoordinatesParser::parseGoogleMaps(const QString &input, double *lon, double *lat,
                                        bool *success) const
{
    // Google Maps schema: -xx.xxxxxxxxxx..., -xx.xxxxxxxxxx...

    const auto match = s_googleMaps.match(input);
    if (! match.hasMatch()) {
        return;
    }

    bool lonOkay = false;
    bool latOkay = false;
    const auto parsedLat = match.captured(1).toDouble(&latOkay);
    const auto parsedLon = match.captured(2).toDouble(&lonOkay);

    if (latOkay && lonOkay) {
        *lon = parsedLon;
        *lat = parsedLat;
        *success = true;
    }
}

void CoordinatesParser::parseOpenStreetMap(const QString &input, double *lon, double *lat,
                                           bool *success) const
{
    // OpenStreetMap Geo-URI schema: geo:-xx.xxxxx,-xx.xxxxx?z=xx

    const auto match = s_openStreetMap.match(input);
    if (! match.hasMatch()) {
        return;
    }

    bool latOkay = false;
    bool lonOkay = false;
    const auto parsedLat = match.captured(1).toDouble(&latOkay);
    const auto parsedLon = match.captured(2).toDouble(&lonOkay);

    if (latOkay && lonOkay) {
        *lon = parsedLon;
        *lat = parsedLat;
        *success = true;
    }
}
