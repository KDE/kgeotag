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

    auto *imagesBox = new QGroupBox(i18n("Images"));
    auto *imagesBoxLayout = new QGridLayout(imagesBox);
    layout->addWidget(imagesBox);

    imagesBoxLayout->addWidget(new QLabel(i18n("Thumbnail size:")), 0, 0);
    m_thumbnailSize = new QSpinBox;
    m_thumbnailSize->setMinimum(16);
    m_thumbnailSize->setMaximum(512);
    m_thumbnailSize->setValue(m_settings->thumbnailSize().width());
    imagesBoxLayout->addWidget(m_thumbnailSize, 0, 1);
    imagesBoxLayout->addWidget(new QLabel(i18n("px")), 0, 2);

    imagesBoxLayout->addWidget(new QLabel(i18n("Preview size:")), 1, 0);
    m_previewSize = new QSpinBox;
    m_previewSize->setMinimum(100);
    m_previewSize->setMaximum(1920);
    m_previewSize->setValue(m_settings->previewSize().width());
    imagesBoxLayout->addWidget(m_previewSize, 1, 1);
    imagesBoxLayout->addWidget(new QLabel(i18n("px")), 1, 2);

    auto *imagesChangesLabel = new QLabel(i18n("Please restart the program after changes to these "
                                               "values so that they are applied and become "
                                               "visible."));
    imagesChangesLabel->setWordWrap(true);
    imagesBoxLayout->addWidget(imagesChangesLabel, 2, 0, 1, 3);

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

    m_lookupElevation = new QCheckBox(i18n("Enable elevation lookup using opentopodata.org\n"
                                           "(requested automatically when placing images on the "
                                           "map)"));
    m_lookupElevation->setChecked(m_settings->lookupElevation());
    assignmentBoxLayout->addWidget(m_lookupElevation, 0, 0, 1, 3);

    auto *exactMatchToleranceLabel = new QLabel(
        i18n("Maximum deviation of the image's time from a GPS point's time for an exact match"));
    exactMatchToleranceLabel->setWordWrap(true);
    assignmentBoxLayout->addWidget(exactMatchToleranceLabel, 1, 0);

    m_exactMatchTolerance = new QSpinBox;
    m_exactMatchTolerance->setMinimum(0);
    m_exactMatchTolerance->setMaximum(300);
    m_exactMatchTolerance->setValue(m_settings->exactMatchTolerance());
    assignmentBoxLayout->addWidget(m_exactMatchTolerance, 1, 1);

    assignmentBoxLayout->addWidget(new QLabel(i18n("seconds")), 1, 2);

    auto *interpolatedMatchLabel = new QLabel(i18n("Boundaries for two coordinates used to "
                                                   "calculate interpolated matches:"));
    interpolatedMatchLabel->setWordWrap(true);
    assignmentBoxLayout->addWidget(interpolatedMatchLabel, 2, 0, 1, 3);

    m_enableMaximumInterpolationInterval = new QCheckBox(i18n("Maximum interval:"));
    assignmentBoxLayout->addWidget(m_enableMaximumInterpolationInterval, 3, 0);

    m_maximumInterpolationInterval = new QSpinBox;
    m_maximumInterpolationInterval->setMinimum(0);
    m_maximumInterpolationInterval->setMaximum(86400);
    assignmentBoxLayout->addWidget(m_maximumInterpolationInterval, 3, 1);

    assignmentBoxLayout->addWidget(new QLabel(i18n("seconds")), 3, 2);

    const int interval = m_settings->maximumInterpolationInterval();
    if (interval == -1) {
        enableMaximumInterpolationInterval(false);
    } else {
        m_enableMaximumInterpolationInterval->setChecked(true);
        m_maximumInterpolationInterval->setValue(interval);
    }

    connect(m_enableMaximumInterpolationInterval, &QCheckBox::toggled,
            this, &SettingsDialog::enableMaximumInterpolationInterval);

    m_enableMaximumInterpolationDistance = new QCheckBox(i18n("Maximum distance:"));
    assignmentBoxLayout->addWidget(m_enableMaximumInterpolationDistance, 4, 0);

    m_maximumInterpolationDistance = new QSpinBox;
    m_maximumInterpolationDistance->setMinimum(0);
    m_maximumInterpolationDistance->setMaximum(1000000);
    assignmentBoxLayout->addWidget(m_maximumInterpolationDistance, 4, 1);

    assignmentBoxLayout->addWidget(new QLabel(i18n("meters")), 4, 2);

    const int distance = m_settings->maximumInterpolationDistance();
    if (distance == -1) {
        enableMaximumInterpolationDistance(false);
    } else {
        m_enableMaximumInterpolationDistance->setChecked(true);
        m_maximumInterpolationDistance->setValue(distance);
    }

    connect(m_enableMaximumInterpolationDistance, &QCheckBox::toggled,
            this, &SettingsDialog::enableMaximumInterpolationDistance);

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

void SettingsDialog::enableMaximumInterpolationDistance(bool state)
{
    m_maximumInterpolationDistance->setEnabled(state);
    if (! state) {
        m_maximumInterpolationDistance->setValue(500);
    }
}

void SettingsDialog::accept()
{
    m_settings->saveThumbnailSize(m_thumbnailSize->value());
    m_settings->savePreviewSize(m_previewSize->value());

    m_settings->saveTrackColor(m_currentTrackColor);
    m_settings->saveTrackWidth(m_trackWidth->value());
    m_settings->saveTrackStyle(static_cast<Qt::PenStyle>(m_trackStyle->currentData().toInt()));

    m_settings->saveExactMatchTolerance(m_exactMatchTolerance->value());
    m_settings->saveMaximumInterpolationInterval(m_enableMaximumInterpolationInterval->isChecked()
        ? m_maximumInterpolationInterval->value() : -1);
    m_settings->saveMaximumInterpolationDistance(m_enableMaximumInterpolationDistance->isChecked()
        ? m_maximumInterpolationDistance->value() : -1);
    m_settings->saveLookupElevation(m_lookupElevation->isChecked());

    m_settings->saveCreateBackups(m_createBackups->isChecked());

    QDialog::accept();
}
