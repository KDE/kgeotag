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

static const QRegularExpression s_googleMaps(
    QStringLiteral("^(-?\\d+\\.\\d+), (-?\\d+\\.\\d+)$"));
static const QRegularExpression s_openStreetMap(
    QStringLiteral("^geo:(-?\\d+\\.\\d+),(-?\\d+\\.\\d+)\\?z=\\d+$"));

CoordinatesParser::CoordinatesParser(QObject *parent, QLocale *locale)
    : QObject(parent),
      m_locale(locale)
{
    // For the human-readable format, localized cardinal directions are used. We cache them here.
    m_n = i18nc("Abbreviated cardinal direction \"North\"", "N");
    m_e = i18nc("Abbreviated cardinal direction \"East\"", "E");
    m_s = i18nc("Abbreviated cardinal direction \"South\"", "S");
    m_w = i18nc("Abbreviated cardinal direction \"West\"", "W");

    // Create a regular expression to look for human-readable coordinates
    const auto coordinate = i18nc("Formatted latitude or longitude with a cardinal direction",
                                  "%1 %2",
                                  QStringLiteral("(\\d+.+)"),
                                  QStringLiteral("([%1])").arg(QRegularExpression::escape(
                                                 m_n + m_e + m_s + m_w)));
    const auto coordinates = i18nc("Formatted coordinates, \"lon, lat\" or \"lat, lon\"", "%1, %2",
                                   coordinate, coordinate);
    m_humanReadable.setPattern(QStringLiteral("^%1$").arg(coordinates));

    qDebug() << m_humanReadable.pattern();
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

    parseHumanReadable(input, &lon, &lat, &success);
    if (success) {
        qCDebug(KGeoTagLog) << "Detected human-readable coordinates with"
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

void CoordinatesParser::parseHumanReadable(const QString &input, double *lon, double *lat,
                                           bool *success) const
{
    const auto match = m_humanReadable.match(input);
    if (! match.hasMatch()) {
        return;
    }

    qCDebug(KGeoTagLog) << input << "could be human-readable coordinates. Continuing ...";
}
