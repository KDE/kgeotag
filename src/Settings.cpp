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

// Local includes
#include "Settings.h"

namespace
{

const QLatin1String c_main("main/");
const QString c_main_windowGeometry = c_main + QLatin1String("window_geometry");
const QString c_main_windowState   = c_main + QLatin1String("window_state");

}

Settings::Settings(QObject *parent)
    : QSettings(QStringLiteral("kgeotag"), QStringLiteral("kgeotag"), parent)
{
}

void Settings::saveMainWindowGeometry(const QByteArray &data)
{
    setValue(c_main_windowGeometry, data);
}

QByteArray Settings::mainWindowGeometry() const
{
    return value(c_main_windowGeometry, QByteArray()).toByteArray();
}

void Settings::saveMainWindowState(const QByteArray &data)
{
    setValue(c_main_windowState, data);
}

QByteArray Settings::mainWindowState() const
{
    return value(c_main_windowState, QByteArray()).toByteArray();
}
