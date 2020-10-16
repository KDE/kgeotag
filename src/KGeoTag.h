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

#ifndef KGEOTAG_H
#define KGEOTAG_H

namespace KGeoTag
{

struct Coordinates
{
    double lon;
    double lat;
    double alt = 0.0;
    bool isSet = true;
};

const Coordinates NoCoordinates { 0.0, 0.0, 0.0, false };

enum MatchType {
    None,
    Exact,
    Interpolated,
    Set
};

}

#endif // KGEOTAG_H
