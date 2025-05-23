// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef COORDINATESFORMATTER_H
#define COORDINATESFORMATTER_H

// Qt includes
#include <QObject>

// Local classes
class Coordinates;
class Settings;

// Qt classes
class QLocale;

class CoordinatesFormatter : public QObject
{
    Q_OBJECT

public:
    explicit CoordinatesFormatter(QObject *parent, QLocale *locale, Settings *settings);
    QString format(const Coordinates &coordinates) const;
    QString alt(const Coordinates &coordinates) const;
    QString formatDecLatLon(const Coordinates &coordinates) const;

private: // Functions
    QString formatLonLat(double value) const;
    QString lon(const Coordinates &coordinates) const;
    QString lat(const Coordinates &coordinates) const;

private: // Variables
    QLocale *m_locale;
    Settings *m_settings;

};

#endif // COORDINATESFORMATTER_H
