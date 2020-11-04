/* Copyright (C) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COORDINATESDIALOG_H
#define COORDINATESDIALOG_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// Qt includes
#include <QDialog>

// Qt classes
class QLineEdit;
class QDoubleSpinBox;

class CoordinatesDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        ManualBookmark,
        EditCoordinates
    };

    explicit CoordinatesDialog(Mode mode, bool hideAlt,
                               const Coordinates &coordinates = Coordinates(),
                               const QString &target = QString());
    QString label() const;
    double lon() const;
    double lat() const;
    double alt() const;
    Coordinates coordinates() const;

private: // Variables
    QLineEdit *m_label;
    QDoubleSpinBox *m_lon;
    QDoubleSpinBox *m_lat;
    QDoubleSpinBox *m_alt;

};

#endif // COORDINATESDIALOG_H
