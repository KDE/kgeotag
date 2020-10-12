/* Copyright (c) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// Local includes
#include "SettingsDialog.h"
#include "Settings.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDebug>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>

SettingsDialog::SettingsDialog(Settings *settings, QWidget *parent)
    : QDialog(parent), m_settings(settings)
{
    setWindowTitle(i18n("KGeoTag: Settings"));

    auto *layout = new QVBoxLayout(this);

    // GPX track rendering

    auto *trackBox = new QGroupBox(i18n("GPX track rendering"));
    auto *trackBoxLayout = new QGridLayout(trackBox);
    layout->addWidget(trackBox);

    trackBoxLayout->addWidget(new QLabel(i18n("Color:")), 0, 0);
    m_trackColor = new QPushButton;
    m_trackColor->setFlat(true);
    m_currentTrackColor = m_settings->trackColor();
    connect(m_trackColor, &QPushButton::clicked, this, &SettingsDialog::setTrackColor);
    trackBoxLayout->addWidget(m_trackColor, 0, 1);

    m_trackOpacity = new QLabel;
    trackBoxLayout->addWidget(m_trackOpacity, 0, 2);

    updateTrackColor();

    trackBoxLayout->addWidget(new QLabel(i18n("Line width:")), 1, 0);
    m_trackWidth = new QSpinBox;
    m_trackWidth->setMinimum(1);
    m_trackWidth->setMaximum(50);
    m_trackWidth->setValue(m_settings->trackWidth());
    trackBoxLayout->addWidget(m_trackWidth, 1, 1, 1, 2);

    trackBoxLayout->addWidget(new QLabel(i18n("Line style:")), 2, 0);
    m_trackStyle = new QComboBox;
    m_trackStyle->addItem(i18n("Solid"), static_cast<int>(Qt::SolidLine));
    m_trackStyle->addItem(i18n("Dashes"), static_cast<int>(Qt::DashLine));
    m_trackStyle->addItem(i18n("Dots"), static_cast<int>(Qt::DotLine));
    m_trackStyle->addItem(i18n("Dash-Dot"), static_cast<int>(Qt::DashDotLine));
    m_trackStyle->addItem(i18n("Dash-Dot-Dot"), static_cast<int>(Qt::DashDotDotLine));
    m_trackStyle->setCurrentIndex(
        m_trackStyle->findData(static_cast<int>(m_settings->trackStyle())));
    trackBoxLayout->addWidget(m_trackStyle, 2, 1, 1, 2);

    // Image assignment

    auto *assignmentBox = new QGroupBox(i18n("Image assignment"));
    auto *assignmentBoxLayout = new QGridLayout(assignmentBox);
    layout->addWidget(assignmentBox);

    auto *exactMatchDeviationLabel = new QLabel(i18n("Maximum deviation (seconds)\n"
                                                     "for an exact match"));
    exactMatchDeviationLabel->setWordWrap(true);
    assignmentBoxLayout->addWidget(exactMatchDeviationLabel, 0, 0);

    m_exactMatchDeviation = new QSpinBox;
    m_exactMatchDeviation->setMinimum(0);
    m_exactMatchDeviation->setMaximum(300);
    m_exactMatchDeviation->setValue(m_settings->exactMatchDeviation());
    assignmentBoxLayout->addWidget(m_exactMatchDeviation, 0, 1);

    m_enableMaximumInterpolationInterval = new QCheckBox(i18n("Maximum interval (seconds)\n"
                                                              "between two points used for\n"
                                                              "an interpolated match"));
    assignmentBoxLayout->addWidget(m_enableMaximumInterpolationInterval, 1, 0);

    m_maximumInterpolationInterval = new QSpinBox;
    m_maximumInterpolationInterval->setMinimum(0);
    m_maximumInterpolationInterval->setMaximum(86400);
    assignmentBoxLayout->addWidget(m_maximumInterpolationInterval, 1, 1);

    const int interval = m_settings->maximumInterpolationInterval();
    if (interval == -1) {
        enableMaximumInterpolationInterval(false);
    } else {
        m_enableMaximumInterpolationInterval->setChecked(true);
        m_maximumInterpolationInterval->setValue(interval);
    }

    connect(m_enableMaximumInterpolationInterval, &QCheckBox::toggled,
            this, &SettingsDialog::enableMaximumInterpolationInterval);

    // Data saving

    auto *saveBox = new QGroupBox(i18n("Saving"));
    auto *saveBoxLayout = new QVBoxLayout(saveBox);
    layout->addWidget(saveBox);

    m_createBackups = new QCheckBox(i18n("Create a backup of each image\n"
                                         "before altering the Exif header"));
    m_createBackups->setChecked(m_settings->createBackups());
    saveBoxLayout->addWidget(m_createBackups);

    // Button box

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void SettingsDialog::updateTrackColor()
{
    m_trackColor->setStyleSheet(QStringLiteral("border: none; background-color: %1;").arg(
                                               m_currentTrackColor.name()));
    m_trackOpacity->setText(i18n("Opacity: %1%", int(m_currentTrackColor.alphaF() * 100.0)));
}

void SettingsDialog::setTrackColor()
{
    QColorDialog dialog(m_currentTrackColor);
    dialog.setOption(QColorDialog::ShowAlphaChannel, true);

    if (! dialog.exec()) {
        return;
    }

    m_currentTrackColor = dialog.currentColor();
    updateTrackColor();
}

void SettingsDialog::enableMaximumInterpolationInterval(bool state)
{
    m_maximumInterpolationInterval->setEnabled(state);
    if (! state) {
        m_maximumInterpolationInterval->setValue(3600);
    }
}

void SettingsDialog::accept()
{
    m_settings->saveTrackColor(m_currentTrackColor);
    m_settings->saveTrackWidth(m_trackWidth->value());
    m_settings->saveTrackStyle(static_cast<Qt::PenStyle>(m_trackStyle->currentData().toInt()));

    m_settings->saveExactMatchDeviation(m_exactMatchDeviation->value());
    m_settings->saveMaximumInterpolationInterval(m_enableMaximumInterpolationInterval->isChecked()
        ? m_maximumInterpolationInterval->value() : -1);

    m_settings->saveCreateBackups(m_createBackups->isChecked());

    QDialog::accept();
}
