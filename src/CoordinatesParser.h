// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef COORDINATESPARSER_H
#define COORDINATESPARSER_H

// Local includes
#include "Coordinates.h"

// Qt includes
#include <QObject>

// Qt classes
class QLocale;

class CoordinatesParser : public QObject
{
    Q_OBJECT

public:
    explicit CoordinatesParser(QObject *parent, QLocale *locale);
    Coordinates parse(const QString &input) const;

private: // Functions
    void parseGoogleMaps(const QString &input, double *lon, double *lat, bool *success) const;
    void parseOpenStreetMap(const QString &input, double *lon, double *lat, bool *success) const;

private: // Variables
    QLocale *m_locale;

};

#endif // COORDINATESPARSER_H
