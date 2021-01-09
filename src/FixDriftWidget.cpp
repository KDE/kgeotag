/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "FixDriftWidget.h"

// Qt includes
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QTimeZone>

// KDE includes
#include <KLocalizedString>

FixDriftWidget::FixDriftWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    auto *timeZoneBox = new QGroupBox(i18n("Images' time zone"));
    auto *timeZoneBoxLayout = new QVBoxLayout(timeZoneBox);
    layout->addWidget(timeZoneBox);

    timeZoneBoxLayout->addWidget(new QLabel(i18n("The images were taken in the following "
                                                 "timezone:")));

    m_timeZone = new QComboBox;
    m_timeZone->setEditable(true);
    m_timeZone->setInsertPolicy(QComboBox::NoInsert);

    const auto systemTimeZoneId = QTimeZone::systemTimeZone().id();
    const auto allTimeZones = QTimeZone::availableTimeZoneIds();
    int index = 0;
    int systemIndex = -1;
    for (const auto &timeZone : allTimeZones) {
        auto currentTimeZone = QTimeZone(timeZone);
        m_timeZone->addItem(QString::fromLatin1(timeZone), timeZone);
        if (systemIndex == -1 && timeZone == systemTimeZoneId) {
            systemIndex = index;
        }
        index++;
    }

    timeZoneBoxLayout->addWidget(m_timeZone);
    if (systemIndex != -1) {
        m_timeZone->setCurrentIndex(systemIndex);
    }

    connect(m_timeZone, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FixDriftWidget::imagesTimeZoneChanged);

    auto *driftBox = new QGroupBox(i18n("Camera clock time drift"));
    auto *driftBoxLayout = new QVBoxLayout(driftBox);
    layout->addWidget(driftBox);

    auto *deviationLayout = new QHBoxLayout;
    driftBoxLayout->addLayout(deviationLayout);

    deviationLayout->addWidget(new QLabel(i18n("Deviation between image dates\n"
                                               "and the (exact) GPS data time:")));

    m_cameraClockDeviation = new QSpinBox;
    m_cameraClockDeviation->setMinimum(-86400);
    m_cameraClockDeviation->setMaximum(86400);
    deviationLayout->addWidget(m_cameraClockDeviation);

    deviationLayout->addWidget(new QLabel(i18n("seconds")));

    deviationLayout->addStretch();

    m_save = new QCheckBox(i18n("Write the fixed dates to the images'\n"
                                "Exif header when saving"));
    driftBoxLayout->addWidget(m_save);

    layout->addStretch();
}

int FixDriftWidget::cameraClockDeviation() const
{
    return m_cameraClockDeviation->value();
}

bool FixDriftWidget::save() const
{
    return m_save->isChecked();
}

QByteArray FixDriftWidget::imagesTimeZoneId() const
{
    return m_timeZone->currentData().toByteArray();
}
