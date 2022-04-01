// SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

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

    const auto systemTimeZoneId = QTimeZone::systemTimeZoneId();
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

    connect(m_timeZone, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FixDriftWidget::imagesTimeZoneChanged);
    connect(m_timeZone, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this]
            {
                m_imagesTimeZone = QTimeZone(m_timeZone->currentData().toByteArray());
            });

    // We should always find a valid index, as QTimeZone::systemTimeZoneId() will return the
    // UTC timezone if the system timezone is not known by Qt. However, to be sure to avoid a
    // segfault:
    if (systemIndex != -1) {
        m_timeZone->setCurrentIndex(systemIndex);
    }

    auto *driftBox = new QGroupBox(i18n("Camera clock time drift"));
    auto *driftBoxLayout = new QVBoxLayout(driftBox);
    layout->addWidget(driftBox);

    auto *driftLabel = new QLabel(i18n("Deviation between image dates and the (exact) GPS data "
                                       "time:"));
    driftLabel->setWordWrap(true);
    driftBoxLayout->addWidget(driftLabel);

    auto *deviationLayout = new QHBoxLayout;
    driftBoxLayout->addLayout(deviationLayout);

    m_driftHours = new QSpinBox;
    m_driftHours->setMinimum(-24);
    m_driftHours->setMaximum(24);
    deviationLayout->addWidget(m_driftHours);
    deviationLayout->addWidget(new QLabel(i18n("hours")));
    connect(m_driftHours, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FixDriftWidget::cameraDriftSettingsChanged);

    m_driftMinutes = new QSpinBox;
    m_driftMinutes->setMinimum(-1440);
    m_driftMinutes->setMaximum(1440);
    deviationLayout->addWidget(m_driftMinutes);
    deviationLayout->addWidget(new QLabel(i18n("minutes")));
    connect(m_driftMinutes, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FixDriftWidget::cameraDriftSettingsChanged);

    m_driftSeconds = new QSpinBox;
    m_driftSeconds->setMinimum(-86400);
    m_driftSeconds->setMaximum(86400);
    deviationLayout->addWidget(m_driftSeconds);
    deviationLayout->addWidget(new QLabel(i18n("seconds")));
    connect(m_driftSeconds, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FixDriftWidget::cameraDriftSettingsChanged);

    deviationLayout->addStretch();

    m_displayFixed = new QCheckBox(i18n("Display the fixed dates and times"));
    m_displayFixed->setChecked(true);
    driftBoxLayout->addWidget(m_displayFixed);
    connect(m_displayFixed, &QCheckBox::toggled, this, &FixDriftWidget::cameraDriftSettingsChanged);

    m_save = new QCheckBox(i18n("Fix the files' dates and times when saving"));
    driftBoxLayout->addWidget(m_save);

    layout->addStretch();
}

int FixDriftWidget::cameraClockDeviation() const
{
    return   m_driftHours->value() * 3600
           + m_driftMinutes->value() * 60
           + m_driftSeconds->value();
}

bool FixDriftWidget::save() const
{
    return m_save->isChecked();
}

QByteArray FixDriftWidget::imagesTimeZoneId() const
{
    return m_timeZone->currentData().toByteArray();
}

const QTimeZone &FixDriftWidget::imagesTimeZone() const
{
    return m_imagesTimeZone;
}

bool FixDriftWidget::setImagesTimeZone(const QByteArray &id)
{
    const int index = m_timeZone->findData(id);
    if (index != -1) {
        m_timeZone->setCurrentIndex(index);
        return true;
    } else {
        return false;
    }
}

bool FixDriftWidget::displayFixed() const
{
    return m_displayFixed->isChecked();
}
