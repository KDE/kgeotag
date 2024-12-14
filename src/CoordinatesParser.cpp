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

    auto ecapedMinDecSec = i18nc("Formatted coordinates as degrees, minutes and decimal seconds",
                                 "%1° %2' %3\"",
                                 QStringLiteral("%1"), QStringLiteral("%2"), QStringLiteral("%3"));
    ecapedMinDecSec.replace(QStringLiteral("\\%1"), QStringLiteral("%1"));
    ecapedMinDecSec.replace(QStringLiteral("\\%2"), QStringLiteral("%2"));
    ecapedMinDecSec.replace(QStringLiteral("\\%3"), QStringLiteral("%3"));

    m_degMinDecSec.setPattern(ecapedMinDecSec.arg(QStringLiteral("(?<degrees>\\d+)"),
                                                  QStringLiteral("(?<minutes>\\d+)"),
                                                  QStringLiteral("(?<seconds>%1)").arg(
                                                                 decimalValue)));
    m_degMinDecSecGroups = m_degMinDecSec.namedCaptureGroups();
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

    auto match = m_humanReadable.match(input);
    if (! match.hasMatch()) {
        return false;
    }

    qCDebug(KGeoTagLog) << input << "could be human-readable coordinates. Continuing ...";

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

    // Check for "degrees, minutes and decimal seconds"

    /*
    match = m_degMinDecSec.match(value1);
    if (match.hasMatch()) {
        int deg = 0;
        int min = 0;
        double sec = 0.0;
        qDebug() << "deg:" << match.captured(m_degMinDecSecGroups.indexOf(QStringLiteral("degrees")));
        qDebug() << "min:" << match.captured(m_degMinDecSecGroups.indexOf(QStringLiteral("minutes")));
        qDebug() << "sec:" << match.captured(m_degMinDecSecGroups.indexOf(QStringLiteral("seconds")));
    }
    */

    return false;
}
