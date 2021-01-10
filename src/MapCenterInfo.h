/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MAPCENTERINFO_H
#define MAPCENTERINFO_H

// Qt includes
#include <QWidget>

// Local classes
class CoordinatesFormatter;

// Marble classes
namespace Marble
{
class GeoDataLatLonAltBox;
}

// Qt classes
class QLabel;

class MapCenterInfo : public QWidget
{
    Q_OBJECT

public:
    explicit MapCenterInfo(CoordinatesFormatter *formatter, QWidget *parent = nullptr);

public slots:
    void mapMoved(const Marble::GeoDataLatLonAltBox &visibleBox);

private: // Variables
    CoordinatesFormatter *m_formatter;
    QLabel *m_label;

};

#endif // MAPCENTERINFO_H
