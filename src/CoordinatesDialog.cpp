// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "CoordinatesDialog.h"
#include "DegreesConverter.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QApplication>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QDebug>
#include <QComboBox>

// C++ includes
#include <cmath>

CoordinatesDialog::CoordinatesDialog(Mode mode, bool hideAlt, bool latBeforeLon,
                                     KGeoTag::CoordinatesFlavor flavor,
                                     const Coordinates &coordinates, const QString &target)
    : QDialog(QApplication::activeWindow()),
      m_flavor(flavor)
{
    auto *layout = new QVBoxLayout(this);
    auto *grid = new QGridLayout;
    layout->addLayout(grid);

    auto *titleLabel = new QLabel;
    titleLabel->setWordWrap(true);

    auto *labelLabel = new QLabel(i18n("Label:"));
    grid->addWidget(labelLabel, 1, 0);
    m_label = new QLineEdit;
    grid->addWidget(m_label, 1, 1);

    const int lonRow = latBeforeLon ? 3 : 2;
    const int latRow = latBeforeLon ? 2 : 3;

    grid->addWidget(new QLabel(i18n("Longitude:")), lonRow, 0);
    m_lonDeg = new QDoubleSpinBox;
    m_lonDeg->setRange(0.0, 180.0);
    m_lonDeg->setSuffix(i18nc("Degrees symbol", "\u2009°"));
    grid->addWidget(m_lonDeg, lonRow, 1);

    m_lonMin = new QDoubleSpinBox;
    m_lonMin->setRange(0.0, 60.0);
    m_lonMin->setSuffix(i18nc("(Degreee) Minutes symbol", "\u2009'"));

    m_lonSec = new QDoubleSpinBox;
    m_lonSec->setRange(0.0, 60.0);
    m_lonSec->setSuffix(i18nc("(Degreee) Seconds symbol", "\u2009\""));

    m_lonDirection = new QComboBox;
    m_lonDirection->addItem(i18nc("Abbreviated cardinal direction \"East\"", "E"));
    m_lonDirection->addItem(i18nc("Abbreviated cardinal direction \"West\"", "W"));

    grid->addWidget(new QLabel(i18n("Latitude:")), latRow, 0);
    m_latDeg = new QDoubleSpinBox;
    m_latDeg->setRange(0.0, 90.0);
    m_latDeg->setSuffix(i18nc("Degrees symbol", "\u2009°"));
    grid->addWidget(m_latDeg, latRow, 1);

    m_latMin = new QDoubleSpinBox;
    m_latMin->setRange(0.0, 60.0);
    m_latMin->setSuffix(i18nc("(Degreee) Minutes symbol", "\u2009'"));

    m_latSec = new QDoubleSpinBox;
    m_latSec->setRange(0.0, 60.0);
    m_latSec->setSuffix(i18nc("(Degreee) Seconds symbol", "\u2009\""));

    m_latDirection = new QComboBox;
    m_latDirection->addItem(i18nc("Abbreviated cardinal direction \"North\"", "N"));
    m_latDirection->addItem(i18nc("Abbreviated cardinal direction \"South\"", "S"));

    auto *altLabel = new QLabel(i18n("Altitude:"));
    grid->addWidget(altLabel, 4, 0);
    altLabel->setVisible(! hideAlt);
    m_alt = new QDoubleSpinBox;
    m_alt->setDecimals(KGeoTag::altitudePrecision);
    m_alt->setRange(KGeoTag::minimalAltitude, KGeoTag::maximalAltitude);
    m_alt->setSuffix(i18nc("Meters abbreviation", "\u2009m"));

    auto *automaticAltLabel = new QLabel(i18n("<i>The altitude is looked up automatically</i>"));
    automaticAltLabel->setWordWrap(true);

    switch (mode) {
    case Mode::ManualBookmark:
        setWindowTitle(i18n("New bookmark"));
        titleLabel->setText(i18n("Data for the new bookmark:"));
        break;

    case Mode::EditCoordinates:
        labelLabel->hide();
        m_label->hide();
        setWindowTitle(i18n("Manual coordinates"));
        titleLabel->setText(i18n("Coordinates for %1:", target));
        break;
    }

    int posSpan;

    switch (m_flavor) {

    case KGeoTag::DecimalDegrees:
        grid->addWidget(titleLabel, 0, 0, 1, 3);

        m_lonDeg->setDecimals(KGeoTag::degreesPrecision);
        m_lonDeg->setValue(std::abs(coordinates.lon()));

        m_latSec->hide();
        m_latDeg->setDecimals(KGeoTag::degreesPrecision);
        m_latDeg->setValue(std::abs(coordinates.lat()));

        posSpan = 2;

        break;

    case KGeoTag::DegreesDecimalMinutes: {
        grid->addWidget(titleLabel, 0, 0, 1, 4);

        double degrees;
        double minutes;

        grid->addWidget(m_lonMin, lonRow, 2);
        grid->addWidget(m_latMin, latRow, 2);

        m_lonDeg->setDecimals(0);
        m_lonMin->setDecimals(KGeoTag::minutesPrecision);
        degrees = DegreesConverter::fromDecimal(std::abs(coordinates.lon()), &minutes);
        m_lonDeg->setValue(degrees);
        m_lonMin->setValue(minutes);

        m_latDeg->setDecimals(0);
        m_latMin->setDecimals(KGeoTag::minutesPrecision);
        degrees = DegreesConverter::fromDecimal(std::abs(coordinates.lat()), &minutes);
        m_latDeg->setValue(degrees);
        m_latMin->setValue(minutes);

        posSpan = 3;

        break; }

    case KGeoTag::DegreesMinutesDecimalSeconds: {
        grid->addWidget(titleLabel, 0, 0, 1, 5);

        double degrees;
        double minutes;
        double seconds;

        grid->addWidget(m_lonMin, lonRow, 2);
        grid->addWidget(m_latMin, latRow, 2);

        grid->addWidget(m_lonSec, lonRow, 3);
        grid->addWidget(m_latSec, latRow, 3);

        m_lonDeg->setDecimals(0);
        m_lonMin->setDecimals(0);
        m_lonSec->setDecimals(KGeoTag::secondsPrecision);
        degrees = DegreesConverter::fromDecimal(std::abs(coordinates.lon()), &minutes, &seconds);
        m_lonDeg->setValue(degrees);
        m_lonMin->setValue(minutes);
        m_lonSec->setValue(seconds);

        m_latDeg->setDecimals(0);
        m_latMin->setDecimals(0);
        m_latSec->setDecimals(KGeoTag::secondsPrecision);
        degrees = DegreesConverter::fromDecimal(std::abs(coordinates.lat()), &minutes, &seconds);
        m_latDeg->setValue(degrees);
        m_latMin->setValue(minutes);
        m_latSec->setValue(seconds);

        posSpan = 4;

        break; }

    }

    grid->addWidget(m_lonDirection, lonRow, posSpan);
    grid->addWidget(m_latDirection, latRow, posSpan);

    grid->addWidget(m_alt, 4, 1, 1, posSpan);
    m_alt->setVisible(! hideAlt);

    grid->addWidget(automaticAltLabel, 5, 0, 1, posSpan + 1);
    automaticAltLabel->setVisible(hideAlt);

    m_lonDirection->setCurrentIndex(coordinates.lon() >= 0 ? 0 : 1);
    m_latDirection->setCurrentIndex(coordinates.lat() >= 0 ? 0 : 1);

    m_alt->setValue(coordinates.alt());

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

QString CoordinatesDialog::label() const
{
    const auto text = m_label->text().simplified();
    return text.isEmpty() ? i18n("Untitled") : text;
}

double CoordinatesDialog::lon() const
{
    double degrees;

    switch (m_flavor) {
    case KGeoTag::DecimalDegrees:
        degrees = m_lonDeg->value();
        break;
    case KGeoTag::DegreesDecimalMinutes:
        degrees = DegreesConverter::toDecimal(m_lonDeg->value(), m_lonMin->value());
        break;
    case KGeoTag::DegreesMinutesDecimalSeconds:
        degrees = DegreesConverter::toDecimal(m_lonDeg->value(), m_lonMin->value(),
                                              m_lonSec->value());
        break;
    }

    return degrees * (m_lonDirection->currentIndex() == 0 ? 1.0 : -1.0);
}

double CoordinatesDialog::lat() const
{
    double degrees;

    switch (m_flavor) {
    case KGeoTag::DecimalDegrees:
        degrees = m_latDeg->value();
        break;
    case KGeoTag::DegreesDecimalMinutes:
        degrees = DegreesConverter::toDecimal(m_latDeg->value(), m_latMin->value());
        break;
    case KGeoTag::DegreesMinutesDecimalSeconds:
        degrees = DegreesConverter::toDecimal(m_latDeg->value(), m_latMin->value(),
                                              m_latSec->value());
        break;
    }

    return degrees * (m_latDirection->currentIndex() == 0 ? 1.0 : -1.0);
}

double CoordinatesDialog::alt() const
{
    return m_alt->value();
}

Coordinates CoordinatesDialog::coordinates() const
{
    return Coordinates(lon(), lat(), m_alt->value(), true);
}
