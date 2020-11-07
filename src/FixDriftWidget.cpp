/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
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
