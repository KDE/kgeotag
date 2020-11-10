/* SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

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

CoordinatesDialog::CoordinatesDialog(Mode mode, bool hideAlt, const Coordinates &coordinates,
                                     const QString &target)
    : QDialog(QApplication::activeWindow())
{
    auto *layout = new QVBoxLayout(this);
    auto *grid = new QGridLayout;
    layout->addLayout(grid);

    int row = -1;

    auto *titleLabel = new QLabel;
    titleLabel->setWordWrap(true);
    grid->addWidget(titleLabel, ++row, 0, 1, 2);

    auto *labelLabel = new QLabel(i18n("Label:"));
    grid->addWidget(labelLabel, ++row, 0);
    m_label = new QLineEdit;
    grid->addWidget(m_label, row, 1);

    auto *lonLabel = new QLabel(i18n("Longitude:"));
    grid->addWidget(lonLabel, ++row, 0);
    m_lon = new QDoubleSpinBox;
    m_lon->setDecimals(KGeoTag::degreesPrecision);
    m_lon->setRange(-180.0, 180.0);
    m_lon->setSuffix(i18nc("Degrees symbol", "\u2009°"));
    m_lon->setValue(coordinates.lon());
    grid->addWidget(m_lon, row, 1);

    auto *latLabel = new QLabel(i18n("Latitude:"));
    grid->addWidget(latLabel, ++row, 0);
    m_lat = new QDoubleSpinBox;
    m_lat->setDecimals(KGeoTag::degreesPrecision);
    m_lat->setRange(-90.0, 90.0);
    m_lat->setSuffix(i18nc("Degrees symbol", "\u2009°"));
    m_lat->setValue(coordinates.lat());
    grid->addWidget(m_lat, row, 1);


    auto *altLabel = new QLabel(i18n("Altitude:"));
    grid->addWidget(altLabel, ++row, 0);
    altLabel->setVisible(! hideAlt);
    m_alt = new QDoubleSpinBox;
    m_alt->setDecimals(KGeoTag::altitudePrecision);
    m_alt->setRange(KGeoTag::minimalAltitude, KGeoTag::maximalAltitude);
    m_alt->setSuffix(i18nc("Meters abbreviation", "\u2009m"));
    m_alt->setValue(coordinates.alt());
    grid->addWidget(m_alt, row, 1);
    m_alt->setVisible(! hideAlt);

    auto *automaticAltLabel = new QLabel(i18n("<i>The altitude is looked up automatically</i>"));
    automaticAltLabel->setWordWrap(true);
    grid->addWidget(automaticAltLabel, ++row, 0, 1, 2);
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
    return m_lon->value();
}

double CoordinatesDialog::lat() const
{
    return m_lat->value();
}

double CoordinatesDialog::alt() const
{
    return m_alt->value();
}

Coordinates CoordinatesDialog::coordinates() const
{
    return Coordinates(m_lon->value(), m_lat->value(), m_alt->value(), true);
}
