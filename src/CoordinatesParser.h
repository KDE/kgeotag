// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef COORDINATESPARSER_H
#define COORDINATESPARSER_H

// Local includes
#include "Coordinates.h"

// Qt includes
#include <QObject>
#include <QRegularExpression>

// Qt classes
class QLocale;

class CoordinatesParser : public QObject
{
    Q_OBJECT

public:
    explicit CoordinatesParser(QObject *parent, QLocale *locale);
    Coordinates parse(const QString &input) const;

private: // Functions
    bool parseGoogleMaps(const QString &input, double *lon, double *lat) const;
    bool parseOpenStreetMap(const QString &input, double *lon, double *lat) const;
    bool parseHumanReadable(const QString &input, double *lon, double *lat) const;
    bool parseDegMinDecimalSec(const QString &input, double *parsed) const;
    bool parseDegDecimalMin(const QString &input, double *parsed) const;
    void assignLonLat(double parsed1, const QString &direction1,
                      double parsed2, const QString &direction2,
                      double *lon, double *lat) const;

private: // Variables
    QLocale *m_locale;
    QString m_n;
    QString m_e;
    QString m_s;
    QString m_w;

    QRegularExpression m_humanReadable;
    QStringList m_humanReadableGroups;
    QRegularExpression m_degMinDecSec;
    QStringList m_degMinDecSecGroups;
    QRegularExpression m_degDecMin;
    QStringList m_degDecMinGroups;
    QRegularExpression m_decDeg;

};

#endif // COORDINATESPARSER_H
