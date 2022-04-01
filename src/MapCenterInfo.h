// SPDX-FileCopyrightText: 2021 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#ifndef MAPCENTERINFO_H
#define MAPCENTERINFO_H

// Qt includes
#include <QWidget>
#include <QDateTime>

// Local classes
class SharedObjects;
class Coordinates;
class CoordinatesFormatter;

// Qt classes
class QLabel;
class QLocale;

class MapCenterInfo : public QWidget
{
    Q_OBJECT

public:
    explicit MapCenterInfo(SharedObjects *sharedObjects, QWidget *parent = nullptr);

public slots:
    void mapMoved(const Coordinates &center);
    void trackPointCentered(const Coordinates &coordinates, const QDateTime &dateTime);

private: // Functions
    void displayCoordinates(const Coordinates &coordinates);

private: // Variables
    CoordinatesFormatter *m_formatter;
    const QLocale *m_locale;
    QLabel *m_coordinatesLabel;
    QLabel *m_dateTimeLabel;

};

#endif // MAPCENTERINFO_H
