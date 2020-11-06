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
#include "AutomaticMatchingWidget.h"
#include "Settings.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QApplication>
#include <QStyle>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDebug>

// C++ includes
#include <functional>

AutomaticMatchingWidget::AutomaticMatchingWidget(Settings *settings, QWidget *parent)
    : QWidget(parent), m_settings(settings)
{
    auto *layout = new QVBoxLayout(this);

    // Exact matching

    auto *exactMatchBox = new QGroupBox(i18n("Search for exact matches"));
    exactMatchBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    auto *exactMatchBoxLayout = new QVBoxLayout(exactMatchBox);
    layout->addWidget(exactMatchBox);

    auto *exactMatchToleranceLabel = new QLabel(i18n(
        "Maximum deviation of the image's time from a GPS point's time to be considered an exact "
        "match:"));
    exactMatchToleranceLabel->setWordWrap(true);
    exactMatchBoxLayout->addWidget(exactMatchToleranceLabel);

    auto *secondsLayout = new QHBoxLayout;
    exactMatchBoxLayout->addLayout(secondsLayout);

    m_exactMatchTolerance = new QSpinBox;
    m_exactMatchTolerance->setMinimum(0);
    m_exactMatchTolerance->setMaximum(300);
    m_exactMatchTolerance->setValue(m_settings->exactMatchTolerance());
    secondsLayout->addWidget(m_exactMatchTolerance);

    secondsLayout->addWidget(new QLabel(i18n("seconds")));

    secondsLayout->addStretch();

    // Interpolated matching

    auto *interpolatedMatchBox = new QGroupBox(i18n("Search for interpolated matches"));
    interpolatedMatchBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    auto *interpolatedMatchBoxLayout = new QVBoxLayout(interpolatedMatchBox);
    layout->addWidget(interpolatedMatchBox);

    auto *interpolatedMatchLabel = new QLabel(i18n(
        "Spatial and temporal boundaries for two coordinates used to calculate an interpolated "
        "match:"));
    interpolatedMatchLabel->setWordWrap(true);
    interpolatedMatchBoxLayout->addWidget(interpolatedMatchLabel);

    auto boundariesWrapperLayout = new QHBoxLayout;
    interpolatedMatchBoxLayout->addLayout(boundariesWrapperLayout);

    auto *boundariesLayout = new QGridLayout;
    boundariesWrapperLayout->addLayout(boundariesLayout);

    m_enableMaximumInterpolationInterval = new QCheckBox(i18n("Maximum interval:"));
    boundariesLayout->addWidget(m_enableMaximumInterpolationInterval, 0, 0);

    m_maximumInterpolationInterval = new QSpinBox;
    m_maximumInterpolationInterval->setMinimum(0);
    m_maximumInterpolationInterval->setMaximum(86400);
    boundariesLayout->addWidget(m_maximumInterpolationInterval, 0, 1);

    boundariesLayout->addWidget(new QLabel(i18n("seconds")), 0, 2);

    const int interval = m_settings->maximumInterpolationInterval();
    if (interval == -1) {
        enableMaximumInterpolationInterval(false);
    } else {
        m_enableMaximumInterpolationInterval->setChecked(true);
        m_maximumInterpolationInterval->setValue(interval);
    }

    connect(m_enableMaximumInterpolationInterval, &QCheckBox::toggled,
            this, &AutomaticMatchingWidget::enableMaximumInterpolationInterval);

    m_enableMaximumInterpolationDistance = new QCheckBox(i18n("Maximum distance:"));
    boundariesLayout->addWidget(m_enableMaximumInterpolationDistance, 1, 0);

    m_maximumInterpolationDistance = new QSpinBox;
    m_maximumInterpolationDistance->setRange(0, 1000000);
    boundariesLayout->addWidget(m_maximumInterpolationDistance, 1, 1);

    boundariesLayout->addWidget(new QLabel(i18n("meters")), 1, 2);

    const int distance = m_settings->maximumInterpolationDistance();
    if (distance == -1) {
        enableMaximumInterpolationDistance(false);
    } else {
        m_enableMaximumInterpolationDistance->setChecked(true);
        m_maximumInterpolationDistance->setValue(distance);
    }

    connect(m_enableMaximumInterpolationDistance, &QCheckBox::toggled,
            this, &AutomaticMatchingWidget::enableMaximumInterpolationDistance);

    boundariesWrapperLayout->addStretch();

    // Bottom buttons

    layout->addStretch();

    auto *buttonsLayout = new QHBoxLayout;
    layout->addLayout(buttonsLayout);

    auto *saveButton = new QPushButton(i18n("Save settings"));
    saveButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
    connect(saveButton, &QPushButton::clicked, this, &AutomaticMatchingWidget::saveSettings);
    buttonsLayout->addWidget(saveButton);

    buttonsLayout->addStretch();

    auto *reassignButton = new QPushButton(i18n("(Re)Assign all images"));
    reassignButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogRetryButton));

    auto *reassignMenu = new QMenu(this);

    auto *combinedSearch = reassignMenu->addAction(i18n("Combined match search"));
    connect(combinedSearch, &QAction::triggered,
            this, std::bind(&AutomaticMatchingWidget::requestReassignment, this,
                            KGeoTag::CombinedMatchSearch));

    reassignMenu->addSeparator();

    auto *exactSearch = reassignMenu->addAction(i18n("Search exact matches only"));
    connect(exactSearch, &QAction::triggered,
            this, std::bind(&AutomaticMatchingWidget::requestReassignment, this,
                            KGeoTag::ExactMatchSearch));

    auto *interpolatedSearch = reassignMenu->addAction(i18n("Search interpolated matches only"));
    connect(interpolatedSearch, &QAction::triggered,
            this, std::bind(&AutomaticMatchingWidget::requestReassignment, this,
                            KGeoTag::InterpolatedMatchSearch));

    reassignMenu->addSeparator();

    m_excludeManuallyTagged = reassignMenu->addAction(i18n("Exclude manually tagged images"));
    m_excludeManuallyTagged->setCheckable(true);
    m_excludeManuallyTagged->setChecked(m_settings->excludeManuallyTaggedWhenReassigning());

    reassignButton->setMenu(reassignMenu);
    buttonsLayout->addWidget(reassignButton);
}

void AutomaticMatchingWidget::enableMaximumInterpolationInterval(bool state)
{
    m_maximumInterpolationInterval->setEnabled(state);
    if (! state) {
        m_maximumInterpolationInterval->setValue(3600);
    }
}

void AutomaticMatchingWidget::enableMaximumInterpolationDistance(bool state)
{
    m_maximumInterpolationDistance->setEnabled(state);
    if (! state) {
        m_maximumInterpolationDistance->setValue(500);
    }
}

void AutomaticMatchingWidget::saveSettings()
{
    m_settings->saveExactMatchTolerance(m_exactMatchTolerance->value());
    m_settings->saveMaximumInterpolationInterval(m_enableMaximumInterpolationInterval->isChecked()
        ? m_maximumInterpolationInterval->value() : -1);
    m_settings->saveMaximumInterpolationDistance(m_enableMaximumInterpolationDistance->isChecked()
        ? m_maximumInterpolationDistance->value() : -1);
    m_settings->saveExcludeManuallyTaggedWhenReassigning(m_excludeManuallyTagged->isChecked());

    QMessageBox::information(this, i18n("Save as default"), i18n("Settings saved!"));
}

bool AutomaticMatchingWidget::excludeManuallyTagged() const
{
    return m_excludeManuallyTagged->isChecked();
}

int AutomaticMatchingWidget::exactMatchTolerance() const
{
    return m_exactMatchTolerance->value();
}

int AutomaticMatchingWidget::maximumInterpolationInterval() const
{
    return m_enableMaximumInterpolationInterval->isChecked()
               ? m_maximumInterpolationInterval->value()
               : -1;
}

int AutomaticMatchingWidget::maximumInterpolationDistance() const
{
    return m_enableMaximumInterpolationDistance->isChecked()
               ? m_maximumInterpolationDistance->value()
               : -1;
}
