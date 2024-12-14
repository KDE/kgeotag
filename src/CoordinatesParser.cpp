// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "CoordinatesParser.h"
#include "DegreesConverter.h"
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
    // CAUTION:
    // Be sure to use the same l10n strings here as are used in the CoordinatesFormatter class,
    // so that we can parse our own formatted "copy to clipboard" coordinates format!

    // For the human-readable format, localized cardinal directions are used. We cache them here.
    m_n = i18nc("Abbreviated cardinal direction \"North\"", "N");
    m_e = i18nc("Abbreviated cardinal direction \"East\"", "E");
    m_s = i18nc("Abbreviated cardinal direction \"South\"", "S");
    m_w = i18nc("Abbreviated cardinal direction \"West\"", "W");

    // Create a regular expression to look for human-readable coordinates

    auto escapedLatLon = QRegularExpression::escape(
        i18nc("Formatted latitude or longitude with a cardinal direction", "%1 %2",
              QStringLiteral("%1"), QStringLiteral("%2")));
    escapedLatLon.replace(QStringLiteral("\\%1"), QStringLiteral("%1"));
    escapedLatLon.replace(QStringLiteral("\\%2"), QStringLiteral("%2"));

    auto escapedCoordinates = QRegularExpression::escape(
        i18nc("Formatted coordinates, \"lon, lat\" or \"lat, lon\"", "%1, %2",
              QStringLiteral("%1"), QStringLiteral("%2")));
    escapedCoordinates.replace(QStringLiteral("\\%1"), QStringLiteral("%1"));
    escapedCoordinates.replace(QStringLiteral("\\%2"), QStringLiteral("%2"));

    const auto escapedNesw = QRegularExpression::escape(m_n + m_e + m_s + m_w);

    const auto coordinate1 = escapedLatLon.arg(QStringLiteral("(?<value1>\\d+.+)"),
                                               QStringLiteral("(?<direction1>[%1])").arg(
                                                              escapedNesw));
    const auto coordinate2 = escapedLatLon.arg(QStringLiteral("(?<value2>\\d+.+)"),
                                               QStringLiteral("(?<direction2>[%1])").arg(
                                                              escapedNesw));
    const auto coordinates = escapedCoordinates.arg(coordinate1, coordinate2);

    m_humanReadable.setPattern(QStringLiteral("^%1$").arg(coordinates));
    m_humanReadableGroups = m_humanReadable.namedCaptureGroups();

    // This will match a decimal value with either "." or "," as the decimal point
    const auto decimalValue = QStringLiteral("\\d+[.,]\\d+");

    // Match degrees, minutes and decimal seconds

    auto ecapedDegMinDecSec = i18nc(
        "Formatted coordinates as degrees, minutes and decimal seconds", "%1° %2' %3\"",
        QStringLiteral("%1"), QStringLiteral("%2"), QStringLiteral("%3"));
    ecapedDegMinDecSec.replace(QStringLiteral("\\%1"), QStringLiteral("%1"));
    ecapedDegMinDecSec.replace(QStringLiteral("\\%2"), QStringLiteral("%2"));
    ecapedDegMinDecSec.replace(QStringLiteral("\\%3"), QStringLiteral("%3"));

    m_degMinDecSec.setPattern(ecapedDegMinDecSec.arg(QStringLiteral("(?<degrees>\\d+)"),
                                                     QStringLiteral("(?<minutes>\\d+)"),
                                                     QStringLiteral("(?<seconds>%1)").arg(
                                                                    decimalValue)));
    m_degMinDecSecGroups = m_degMinDecSec.namedCaptureGroups();

    // Match degrees and decimal minutes

    auto escapedDegDecMin = i18nc(
        "Formatted coordinates as degrees and decimal minutes", "%1° %2'",
        QStringLiteral("%1"), QStringLiteral("%2"));
    escapedDegDecMin.replace(QStringLiteral("\\%1"), QStringLiteral("%1"));
    escapedDegDecMin.replace(QStringLiteral("\\%2"), QStringLiteral("%2"));

    m_degDecMin.setPattern(escapedDegDecMin.arg(QStringLiteral("(?<degrees>\\d+)"),
                                                QStringLiteral("(?<minutes>%1)").arg(
                                                               decimalValue)));
    m_degDecMinGroups = m_degDecMin.namedCaptureGroups();

    // Match decimal degrees
    auto escapedDecDeg = i18nc("Formatted coordinates as decimal degrees", "%1°",
                               QStringLiteral("%1"));
    escapedDecDeg.replace(QStringLiteral("\\%1"), QStringLiteral("%1"));
    m_decDeg.setPattern(escapedDecDeg.arg(QStringLiteral("(%1)").arg(decimalValue)));
}

Coordinates CoordinatesParser::parse(const QString &input) const
{
    qCDebug(KGeoTagLog) << "Parsing coordinates string" << input;
    double lon = 0.0;
    double lat = 0.0;

    if (parseGoogleMaps(input, &lon, &lat)) {
        qCDebug(KGeoTagLog) << "Detected Google Maps coordinates with"
                            << "lon" << lon << "and lat" << lat;
        return Coordinates(lon, lat, 0.0, true);
    }

    if (parseOpenStreetMap(input, &lon, &lat)) {
        qCDebug(KGeoTagLog) << "Detected OpenStreetMap coordinates with"
                            << "lon" << lon << "and lat" << lat;
        return Coordinates(lon, lat, 0.0, true);
    }

    if (parseHumanReadable(input, &lon, &lat)) {
        qCDebug(KGeoTagLog) << "Detected human-readable coordinates with"
                            << "lon" << lon << "and lat" << lat;
        return Coordinates(lon, lat, 0.0, true);
    }

    qCDebug(KGeoTagLog) << "Could not extract coordinates from" << input;
    return Coordinates();
}

bool CoordinatesParser::parseGoogleMaps(const QString &input, double *lon, double *lat) const
{
    // Google Maps schema: -xx.xxxxxxxxxx..., -xx.xxxxxxxxxx...

    const auto match = s_googleMaps.match(input);
    if (! match.hasMatch()) {
        return false;
    }

    bool lonOkay = false;
    bool latOkay = false;
    const auto parsedLat = match.captured(1).toDouble(&latOkay);
    const auto parsedLon = match.captured(2).toDouble(&lonOkay);

    if (latOkay && lonOkay) {
        *lon = parsedLon;
        *lat = parsedLat;
        return true;
    }

    return false;
}

bool CoordinatesParser::parseOpenStreetMap(const QString &input, double *lon, double *lat) const
{
    // OpenStreetMap Geo-URI schema: geo:-xx.xxxxx,-xx.xxxxx?z=xx

    const auto match = s_openStreetMap.match(input);
    if (! match.hasMatch()) {
        return false;
    }

    bool latOkay = false;
    bool lonOkay = false;
    const auto parsedLat = match.captured(1).toDouble(&latOkay);
    const auto parsedLon = match.captured(2).toDouble(&lonOkay);

    if (latOkay && lonOkay) {
        *lon = parsedLon;
        *lat = parsedLat;
        return true;
    }

    return false;
}

bool CoordinatesParser::parseHumanReadable(const QString &input, double *lon, double *lat) const
{
    // Search for the human-readable format KGeoTag copies to the clipboard.
    // This is a bit harder ;-)

    const auto match = m_humanReadable.match(input);
    if (! match.hasMatch()) {
        return false;
    }

    qCDebug(KGeoTagLog) << "    The input could be human-readable coordinates. Continuing ...";

    // First we check if the captured cardinal directions are plausible

    const auto direction1 = match.captured(m_humanReadableGroups.indexOf(
        QStringLiteral("direction1")));
    const auto direction2 = match.captured(m_humanReadableGroups.indexOf(
        QStringLiteral("direction2")));

    if (   ! (   (direction1 == m_n || direction1 == m_s)
              && (direction2 == m_e || direction2 == m_w))
        && ! (   (direction1 == m_e || direction1 == m_w)
              && (direction2 == m_n || direction2 == m_s))) {

        qCDebug(KGeoTagLog) << "Cardinal directions" << direction1 << "and" << direction2
                            << "are not plausible";
        return false;
    }

    // The directions are okay. Cache the corresponding values
    const auto value1 = match.captured(m_humanReadableGroups.indexOf(
        QStringLiteral("value1")));
    const auto value2 = match.captured(m_humanReadableGroups.indexOf(
        QStringLiteral("value2")));

    double parsed1 = 0.0;
    double parsed2 = 0.0;

    // Check for "degrees, minutes and decimal seconds"
    if (parseDegMinDecSec(value1, &parsed1) && parseDegMinDecSec(value2, &parsed2)) {
        qCDebug(KGeoTagLog) << "    Found \"degress, minutes and decimal seconds\" format";
        assignLonLat(parsed1, direction1, parsed2, direction2, lon, lat);
        return true;
    }

    // Check for "degrees and decimal minutes"
    if (parseDegDecMin(value1, &parsed1) && parseDegDecMin(value2, &parsed2)) {
        qCDebug(KGeoTagLog) << "    Found \"degress and decimal minutes\" format";
        assignLonLat(parsed1, direction1, parsed2, direction2, lon, lat);
        return true;
    }

    // Check for "decimal degrees"
    if (parseDecDeg(value1, &parsed1) && parseDecDeg(value2, &parsed2)) {
        qCDebug(KGeoTagLog) << "    Found \"decimal degress\" format";
        assignLonLat(parsed1, direction1, parsed2, direction2, lon, lat);
        return true;
    }

    return false;
}

bool CoordinatesParser::parseDegMinDecSec(const QString &input, double *parsed) const
{
    const auto match = m_degMinDecSec.match(input);
    if (! match.hasMatch()) {
        return false;
    }

    bool okay = false;

    const auto deg = match.captured(m_degMinDecSecGroups.indexOf(
                                        QStringLiteral("degrees"))).toInt(&okay);
    if (! okay) {
        return false;
    }

    const auto min = match.captured(m_degMinDecSecGroups.indexOf(
                                        QStringLiteral("minutes"))).toInt(&okay);
    if (! okay) {
        return false;
    }

    const auto sec = m_locale->toDouble(
        match.captured(m_degMinDecSecGroups.indexOf(QStringLiteral("seconds"))), &okay);
    if (! okay) {
        return false;
    }

    *parsed = DegreesConverter::toDecimal(deg, min, sec);
    return true;
}

bool CoordinatesParser::parseDegDecMin(const QString &input, double *parsed) const
{
    const auto match = m_degDecMin.match(input);
    if (! match.hasMatch()) {
        return false;
    }

    bool okay = false;

    const auto deg = match.captured(m_degDecMinGroups.indexOf(
                                        QStringLiteral("degrees"))).toInt(&okay);
    if (! okay) {
        return false;
    }

    const auto min = m_locale->toDouble(
        match.captured(m_degDecMinGroups.indexOf(QStringLiteral("minutes"))), &okay);
    if (! okay) {
        return false;
    }

    *parsed = DegreesConverter::toDecimal(deg, min);
    return true;
}

bool CoordinatesParser::parseDecDeg(const QString &input, double *parsed) const
{
    const auto match = m_decDeg.match(input);
    if (! match.hasMatch()) {
        return false;
    }

    bool okay = false;
    const auto deg = m_locale->toDouble(match.captured(1), &okay);
    if (! okay) {
        return false;
    }

    *parsed = deg;
    return true;
}

void CoordinatesParser::assignLonLat(double parsed1, const QString &direction1,
                                     double parsed2, const QString &direction2,
                                     double *lon, double *lat) const
{
    if (direction1 == m_s || direction1 == m_w) {
        parsed1 *= -1;
    }
    if (direction2 == m_s || direction2 == m_w) {
        parsed2 *= -1;
    }
    if (direction1 == m_n || direction1 == m_s) {
        *lon = parsed2;
        *lat = parsed1;
    } else {
        *lon = parsed1;
        *lat = parsed2;
    }
}
