/* Copyright (c) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef MAPWIDGET_H
#define MAPWIDGET_H

// Local includes
#include "Coordinates.h"

// Qt includes
#include <QWidget>

namespace Marble
{
class MarbleWidget;
class GeoDataCoordinates;
}

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    QHash<QString, bool> floatersVisibility() const;
    void setFloatersVisibility(const QHash<QString, bool> &data);
    Coordinates::Data mapCenter() const;
    void setCenter(const Coordinates::Data &coordinates);
    int zoom() const;
    void setZoom(int zoom);

private: // Functions
    Coordinates::Data toCoordinates(const Marble::GeoDataCoordinates &data) const;

private: // Variables
    Marble::MarbleWidget *m_mapWidget;

};

#endif // MAPWIDGET_H
