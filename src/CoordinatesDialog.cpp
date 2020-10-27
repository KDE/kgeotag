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

// Local includes
#include "CoordinatesDialog.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>

CoordinatesDialog::CoordinatesDialog(Mode mode, bool hideAlt,
                                     const KGeoTag::Coordinates &coordinates, QWidget *parent)
    : QDialog(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *grid = new QGridLayout;
    layout->addLayout(grid);

    int row = 0;

    auto *titleLabel = new QLabel;
    titleLabel->setWordWrap(true);
    grid->addWidget(titleLabel, row++, 0, 1, 2);

    auto *labelLabel = new QLabel(i18n("Label:"));
    grid->addWidget(labelLabel, row, 0);
    m_label = new QLineEdit;
    grid->addWidget(m_label, row++, 1);

    auto *lonLabel = new QLabel(i18n("Longitude:"));
    grid->addWidget(lonLabel, row, 0);
    m_lon = new QDoubleSpinBox;
    m_lon->setDecimals(KGeoTag::degreesPrecision);
    m_lon->setMinimum(-180.0);
    m_lon->setMaximum(180.0);
    m_lon->setValue(coordinates.lon);
    grid->addWidget(m_lon, row++, 1);

    auto *latLabel = new QLabel(i18n("Latitude:"));
    grid->addWidget(latLabel, row, 0);
    m_lat = new QDoubleSpinBox;
    m_lat->setDecimals(KGeoTag::degreesPrecision);
    m_lat->setMinimum(-180.0);
    m_lat->setMaximum(180.0);
    m_lon->setValue(coordinates.lat);
    grid->addWidget(m_lat, row++, 1);

    auto *altLabel = new QLabel(i18n("Altitude:"));
    grid->addWidget(altLabel, row, 0);
    m_alt = new QDoubleSpinBox;
    m_alt->setDecimals(KGeoTag::altitudePrecision);
    m_alt->setMinimum(KGeoTag::minimalAltitude);
    m_alt->setMaximum(KGeoTag::maximalAltitude);
    m_lon->setValue(coordinates.alt);
    grid->addWidget(m_alt, row++, 1);

    switch (mode) {
    case Mode::ManualBookmark:
        setWindowTitle(i18n("New bookmark"));
        titleLabel->setText(i18n("Data for the new bookmark:"));
        break;
    }

    if (hideAlt) {
        altLabel->hide();
        m_alt->hide();
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
