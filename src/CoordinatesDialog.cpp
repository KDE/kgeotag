// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "CoordinatesDialog.h"

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
    grid->addWidget(titleLabel, 0, 0, 1, 2);

    auto *labelLabel = new QLabel(i18n("Label:"));
    grid->addWidget(labelLabel, 1, 0);
    m_label = new QLineEdit;
    grid->addWidget(m_label, 1, 1);

    const int lonRow = latBeforeLon ? 3 : 2;
    const int latRow = latBeforeLon ? 2 : 3;

    grid->addWidget(new QLabel(i18n("Longitude:")), lonRow, 0);
    m_lonDeg = new QDoubleSpinBox;
    m_lonDeg->setDecimals(KGeoTag::degreesPrecision);
    m_lonDeg->setRange(-180.0, 180.0);
    m_lonDeg->setSuffix(i18nc("Degrees symbol", "\u2009°"));
    m_lonDeg->setValue(coordinates.lon());
    grid->addWidget(m_lonDeg, lonRow, 1);

    grid->addWidget(new QLabel(i18n("Latitude:")), latRow, 0);
    m_latDeg = new QDoubleSpinBox;
    m_latDeg->setDecimals(KGeoTag::degreesPrecision);
    m_latDeg->setRange(-90.0, 90.0);
    m_latDeg->setSuffix(i18nc("Degrees symbol", "\u2009°"));
    m_latDeg->setValue(coordinates.lat());
    grid->addWidget(m_latDeg, latRow, 1);

    auto *altLabel = new QLabel(i18n("Altitude:"));
    grid->addWidget(altLabel, 4, 0);
    altLabel->setVisible(! hideAlt);
    m_alt = new QDoubleSpinBox;
    m_alt->setDecimals(KGeoTag::altitudePrecision);
    m_alt->setRange(KGeoTag::minimalAltitude, KGeoTag::maximalAltitude);
    m_alt->setSuffix(i18nc("Meters abbreviation", "\u2009m"));
    m_alt->setValue(coordinates.alt());
    grid->addWidget(m_alt, 4, 1);
    m_alt->setVisible(! hideAlt);

    auto *automaticAltLabel = new QLabel(i18n("<i>The altitude is looked up automatically</i>"));
    automaticAltLabel->setWordWrap(true);
    grid->addWidget(automaticAltLabel, 5, 0, 1, 2);
    automaticAltLabel->setVisible(hideAlt);

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
    return m_lonDeg->value();
}

double CoordinatesDialog::lat() const
{
    return m_latDeg->value();
}

double CoordinatesDialog::alt() const
{
    return m_alt->value();
}

Coordinates CoordinatesDialog::coordinates() const
{
    return Coordinates(lon(), lat(), m_alt->value(), true);
}
