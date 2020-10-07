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

#ifndef SETTINGS_H
#define SETTINGS_H

// Qt includes
#include <QSettings>

class Settings : public QSettings
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent);

    void saveMainWindowGeometry(const QByteArray &data);
    QByteArray mainWindowGeometry() const;

    void saveMainWindowState(const QByteArray &data);
    QByteArray mainWindowState() const;

    void saveFloatersVisibility(const QHash<QString, bool> &data);
    QHash<QString, bool> floatersVisibility();

};

#endif // SETTINGS_H
