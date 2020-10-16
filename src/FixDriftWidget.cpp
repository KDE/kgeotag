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
#include "FixDriftWidget.h"

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>

// KDE includes
#include <KLocalizedString>

FixDriftWidget::FixDriftWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    auto *deviationLayout = new QHBoxLayout;
    layout->addLayout(deviationLayout);

    deviationLayout->addWidget(new QLabel(i18n("Deviation between image dates\n"
                                               "and the (exact) GPS data time:")));

    m_deviation = new QSpinBox;
    m_deviation->setMinimum(-86400);
    m_deviation->setMaximum(86400);
    deviationLayout->addWidget(m_deviation);

    deviationLayout->addWidget(new QLabel(i18n("seconds")));

    deviationLayout->addStretch();

    m_save = new QCheckBox(i18n("Write the fixed dates to the images'\n"
                                "Exif header when saving"));
    layout->addWidget(m_save);

    layout->addStretch();
}

int FixDriftWidget::deviation() const
{
    return m_deviation->value();
}

bool FixDriftWidget::save() const
{
    return m_save->isChecked();
}
