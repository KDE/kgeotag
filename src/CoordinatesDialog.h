// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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

    explicit CoordinatesDialog(Mode mode, bool hideAlt, bool latBeforeLon,
                               KGeoTag::CoordinatesFlavor flavor,
                               const Coordinates &coordinates = Coordinates(),
                               const QString &target = QString());
    QString label() const;
    double lon() const;
    double lat() const;
    double alt() const;
    Coordinates coordinates() const;

private: // Variables
    KGeoTag::CoordinatesFlavor m_flavor;
    QLineEdit *m_label;

    QDoubleSpinBox *m_lonDeg;
    QDoubleSpinBox *m_lonMin;
    QDoubleSpinBox *m_lonSec;

    QDoubleSpinBox *m_latDeg;
    QDoubleSpinBox *m_latMin;
    QDoubleSpinBox *m_latSec;

    QDoubleSpinBox *m_alt;

};

#endif // COORDINATESDIALOG_H
