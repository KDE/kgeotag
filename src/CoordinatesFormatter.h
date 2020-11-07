/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef COORDINATESFORMATTER_H
#define COORDINATESFORMATTER_H

// Qt includes
#include <QObject>

// Local classes
class Coordinates;

// Qt classes
class QLocale;

class CoordinatesFormatter : public QObject
{
    Q_OBJECT

public:
    explicit CoordinatesFormatter(QObject *parent, QLocale *locale);
    QString lon(const Coordinates &coordinates) const;
    QString lat(const Coordinates &coordinates) const;
    QString alt(const Coordinates &coordinates) const;

private: // Functions
    QString formatLonLat(double value) const;

private: // Variables
    QLocale *m_locale;

};

#endif // COORDINATESFORMATTER_H
