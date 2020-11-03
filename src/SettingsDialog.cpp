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
#include <QScrollArea>
#include <QScrollBar>

SettingsDialog::SettingsDialog(Settings *settings, QWidget *parent)
    : QDialog(parent), m_settings(settings)
{
    setWindowTitle(i18n("KGeoTag: Settings"));

    auto *mainLayout = new QVBoxLayout(this);

    // Header

    auto *header = new QLabel(i18n("KGeoTag settings"));
    header->setStyleSheet(QStringLiteral("QLabel { font-weight: bold; font-size: %1pt; }").arg(
        (int) double(header->font().pointSize()) * 1.2));
    header->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(header);

    // All settings

    setStyleSheet(QStringLiteral("QGroupBox { font-weight: bold; }"));

    auto *settingsWidget = new QWidget;

    int row;

    auto *layout = new QVBoxLayout(settingsWidget);

    auto *imagesBox = new QGroupBox(i18n("Images"));
    auto *imagesBoxLayout = new QGridLayout(imagesBox);
    row = -1;
    layout->addWidget(imagesBox);

    m_splitImagesList = new QCheckBox(i18n("Split the images list to an \"Unassigned images\"\n"
                                           "and an \"Assigned images\" list"));
    m_splitImagesList->setChecked(m_settings->splitImagesList());
    imagesBoxLayout->addWidget(m_splitImagesList, ++row, 0, 1, 3);

    imagesBoxLayout->addWidget(new QLabel(i18n("Thumbnail size:")), ++row, 0);
    m_thumbnailSize = new QSpinBox;
    m_thumbnailSize->setMinimum(16);
    m_thumbnailSize->setMaximum(512);
    m_thumbnailSize->setValue(m_settings->thumbnailSize().width());
    imagesBoxLayout->addWidget(m_thumbnailSize, row, 1);
    imagesBoxLayout->addWidget(new QLabel(i18n("px")), row, 2);

    imagesBoxLayout->addWidget(new QLabel(i18n("Preview size:")), ++row, 0);
    m_previewSize = new QSpinBox;
    m_previewSize->setMinimum(100);
    m_previewSize->setMaximum(1920);
    m_previewSize->setValue(m_settings->previewSize().width());
    imagesBoxLayout->addWidget(m_previewSize, row, 1);
    imagesBoxLayout->addWidget(new QLabel(i18n("px")), row, 2);

    auto *imagesChangesLabel = new QLabel(i18n("Please restart the program after changes to these "
                                               "values so that they are applied and become "
                                               "visible."));
    imagesChangesLabel->setWordWrap(true);
    imagesBoxLayout->addWidget(imagesChangesLabel, ++row, 0, 1, 3);

    // GPX track rendering

    auto *trackBox = new QGroupBox(i18n("GPX track rendering"));
    auto *trackBoxLayout = new QGridLayout(trackBox);
    row = -1;
    layout->addWidget(trackBox);

    trackBoxLayout->addWidget(new QLabel(i18n("Color:")), ++row, 0);
    m_trackColor = new QPushButton;
    m_trackColor->setFlat(true);
    m_currentTrackColor = m_settings->trackColor();
    connect(m_trackColor, &QPushButton::clicked, this, &SettingsDialog::setTrackColor);
    trackBoxLayout->addWidget(m_trackColor, row, 1);

    m_trackOpacity = new QLabel;
    trackBoxLayout->addWidget(m_trackOpacity, row, 2);

    updateTrackColor();

    trackBoxLayout->addWidget(new QLabel(i18n("Line width:")), ++row, 0);
    m_trackWidth = new QSpinBox;
    m_trackWidth->setMinimum(1);
    m_trackWidth->setMaximum(50);
    m_trackWidth->setValue(m_settings->trackWidth());
    trackBoxLayout->addWidget(m_trackWidth, row, 1);

    trackBoxLayout->addWidget(new QLabel(i18n("Line style:")), ++row, 0);
    m_trackStyle = new QComboBox;
    m_trackStyle->addItem(i18n("Solid"), static_cast<int>(Qt::SolidLine));
    m_trackStyle->addItem(i18n("Dashes"), static_cast<int>(Qt::DashLine));
    m_trackStyle->addItem(i18n("Dots"), static_cast<int>(Qt::DotLine));
    m_trackStyle->addItem(i18n("Dash-Dot"), static_cast<int>(Qt::DashDotLine));
    m_trackStyle->addItem(i18n("Dash-Dot-Dot"), static_cast<int>(Qt::DashDotDotLine));
    m_trackStyle->setCurrentIndex(
        m_trackStyle->findData(static_cast<int>(m_settings->trackStyle())));
    trackBoxLayout->addWidget(m_trackStyle, row, 1);

    // Image assignment

    auto *assignmentBox = new QGroupBox(i18n("Automatic image assignment"));
    auto *assignmentBoxLayout = new QGridLayout(assignmentBox);
    row = -1;
    layout->addWidget(assignmentBox);

    auto *exactMatchToleranceLabel = new QLabel(
        i18n("Maximum deviation of the image's time from a GPS point's time for an exact match"));
    exactMatchToleranceLabel->setWordWrap(true);
    assignmentBoxLayout->addWidget(exactMatchToleranceLabel, ++row, 0);

    m_exactMatchTolerance = new QSpinBox;
    m_exactMatchTolerance->setMinimum(0);
    m_exactMatchTolerance->setMaximum(300);
    m_exactMatchTolerance->setValue(m_settings->exactMatchTolerance());
    assignmentBoxLayout->addWidget(m_exactMatchTolerance, row, 1);

    assignmentBoxLayout->addWidget(new QLabel(i18n("seconds")), row, 2);

    auto *interpolatedMatchLabel = new QLabel(i18n("Boundaries for two coordinates used to "
                                                   "calculate interpolated matches:"));
    interpolatedMatchLabel->setWordWrap(true);
    assignmentBoxLayout->addWidget(interpolatedMatchLabel, ++row, 0, 1, 3);

    m_enableMaximumInterpolationInterval = new QCheckBox(i18n("Maximum interval:"));
    assignmentBoxLayout->addWidget(m_enableMaximumInterpolationInterval, ++row, 0);

    m_maximumInterpolationInterval = new QSpinBox;
    m_maximumInterpolationInterval->setMinimum(0);
    m_maximumInterpolationInterval->setMaximum(86400);
    assignmentBoxLayout->addWidget(m_maximumInterpolationInterval, row, 1);

    assignmentBoxLayout->addWidget(new QLabel(i18n("seconds")), row, 2);

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
    assignmentBoxLayout->addWidget(m_enableMaximumInterpolationDistance, ++row, 0);

    m_maximumInterpolationDistance = new QSpinBox;
    m_maximumInterpolationDistance->setRange(0, 1000000);
    assignmentBoxLayout->addWidget(m_maximumInterpolationDistance, row, 1);

    assignmentBoxLayout->addWidget(new QLabel(i18n("meters")), row, 2);

    const int distance = m_settings->maximumInterpolationDistance();
    if (distance == -1) {
        enableMaximumInterpolationDistance(false);
    } else {
        m_enableMaximumInterpolationDistance->setChecked(true);
        m_maximumInterpolationDistance->setValue(distance);
    }

    connect(m_enableMaximumInterpolationDistance, &QCheckBox::toggled,
            this, &SettingsDialog::enableMaximumInterpolationDistance);

    // Elevation lookup

    auto *elevationBox = new QGroupBox(i18n("Elevation lookup"));
    auto *elevationBoxLayout = new QGridLayout(elevationBox);
    row = -1;
    layout->addWidget(elevationBox);

    m_lookupElevation = new QCheckBox(i18n("Request and set altitudes automatically\n"
                                           "using opentopodata.org's web API"));
    m_lookupElevation->setChecked(m_settings->lookupElevation());
    elevationBoxLayout->addWidget(m_lookupElevation, ++row, 0, 1, 2);

    elevationBoxLayout->addWidget(new QLabel(i18n("Elevation dataset:")), ++row, 0);

    m_elevationDataset = new QComboBox;
    connect(m_lookupElevation, &QCheckBox::toggled, m_elevationDataset, &QWidget::setEnabled);
    m_elevationDataset->setEnabled(m_settings->lookupElevation());
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset", "ASTER"),
                                QStringLiteral("aster30m"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset", "ETOPO1"),
                                QStringLiteral("etopo1"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset", "EU-DEM"),
                                QStringLiteral("eudem25m"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset", "Mapzen"),
                                QStringLiteral("mapzen"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset", "NED"),
                                QStringLiteral("ned10m"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset", "NZ DEM"),
                                QStringLiteral("nzdem8m"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset", "SRTM (30 m)"),
                                QStringLiteral("srtm30m"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset", "SRTM (90 m)"),
                                QStringLiteral("srtm90m"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset",
                                      "EMODnet 2018 Bathymetry"),
                                QStringLiteral("emod2018"));
    m_elevationDataset->addItem(i18nc("opentopodata.org elevation dataset",
                                      "GEBCO 2020 Bathymetry"),
                                QStringLiteral("gebco2020"));
    elevationBoxLayout->addWidget(m_elevationDataset, row, 1);

    m_elevationDataset->setCurrentIndex(
        m_elevationDataset->findData(m_settings->elevationDataset()));

    auto *datasetInfoLabel = new QLabel(i18n("Cf. <a href=\"https://www.opentopodata.org/\">"
                                             "https://www.opentopodata.org/</a> for further "
                                             "information about the available datasets, like the "
                                             "respective coverage!"));
    datasetInfoLabel->setWordWrap(true);
    datasetInfoLabel->setOpenExternalLinks(true);
    elevationBoxLayout->addWidget(datasetInfoLabel, ++row, 0, 1, 2);

    // Data saving

    auto *saveBox = new QGroupBox(i18n("Saving"));
    auto *saveBoxLayout = new QGridLayout(saveBox);
    row = -1;
    layout->addWidget(saveBox);

    saveBoxLayout->addWidget(new QLabel(i18n("Write changes to:")), ++row, 0);

    m_writeMode = new QComboBox;
    m_writeMode->addItem(i18n("Exif header"),
                         QStringLiteral("WRITETOIMAGEONLY"));
    m_writeMode->addItem(i18n("XMP sidecar file"),
                         QStringLiteral("WRITETOSIDECARONLY"));
    m_writeMode->addItem(i18n("Exif header and XMP sidecar file"),
                         QStringLiteral("WRITETOSIDECARANDIMAGE"));
    saveBoxLayout->addWidget(m_writeMode, row, 1);

    m_writeMode->setCurrentIndex(m_writeMode->findData(m_settings->writeMode()));

    m_createBackups = new QCheckBox(i18n("Create a backups before altering a file"));
    m_createBackups->setChecked(m_settings->createBackups());
    saveBoxLayout->addWidget(m_createBackups, ++row, 0, 1, 2);

    // Scroll area

    auto *scrollArea = new QScrollArea;
    const int styleAddition = scrollArea->width() - scrollArea->viewport()->width();
    scrollArea->setWidget(settingsWidget);
    mainLayout->addWidget(scrollArea);

    show();
    const int widgetWidth = settingsWidget->width() + scrollArea->verticalScrollBar()->width()
                            + styleAddition;
    scrollArea->setMinimumWidth(widgetWidth);
    scrollArea->setMaximumWidth(widgetWidth);
    setMaximumWidth(width());

    // Button box

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
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
    m_settings->saveSplitImagesList(m_splitImagesList->isChecked());
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
    m_settings->saveElevationDataset(m_elevationDataset->currentData().toString());

    m_settings->saveWriteMode(m_writeMode->currentData().toString());
    m_settings->saveCreateBackups(m_createBackups->isChecked());

    QDialog::accept();
}
