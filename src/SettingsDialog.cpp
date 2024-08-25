// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include <QCheckBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QApplication>

SettingsDialog::SettingsDialog(Settings *settings, QWidget *parent)
    : QDialog(parent),
      m_settings(settings)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(i18n("KGeoTag: Settings"));

    auto *mainLayout = new QVBoxLayout(this);

    // Header

    auto *header = new QLabel(i18n("KGeoTag settings"));
    header->setStyleSheet(QStringLiteral("QLabel { font-weight: bold; font-size: %1pt; }").arg(
        (int) double(header->font().pointSize()) * 1.2));
    header->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(header);

    // Settings

    setStyleSheet(QStringLiteral("QGroupBox { font-weight: bold; }"));

    auto *settingsWidget = new QWidget;

    int row;

    auto *layout = new QVBoxLayout(settingsWidget);

    // Coordinates

    auto *coordinatesBox = new QGroupBox(i18n("Coordinates"));
    layout->addWidget(coordinatesBox);

    auto *coordinatesBoxLayout = new QVBoxLayout(coordinatesBox);

    auto *coordinatesBoxWrapper = new QHBoxLayout;
    coordinatesBoxLayout->addLayout(coordinatesBoxWrapper);

    auto *coordinatesSettingsLayout = new QGridLayout;
    coordinatesBoxWrapper->addLayout(coordinatesSettingsLayout);
    coordinatesBoxWrapper->addStretch();

    coordinatesSettingsLayout->addWidget(new QLabel(i18n("Coordinates order:")), 0, 0);
    m_coordinatesOrder = new QComboBox;
    m_coordinatesOrder->addItem(i18n("Longitude, latitude"));
    m_coordinatesOrder->addItem(i18n("Latitude, longitude"));
    m_coordinatesOrder->setCurrentIndex(m_settings->latBeforeLon());
    coordinatesSettingsLayout->addWidget(m_coordinatesOrder, 0, 1);

    coordinatesSettingsLayout->addWidget(new QLabel(i18n("Coordinates flavor:")), 1, 0);
    m_coordinatesFlavor = new QComboBox;
    m_coordinatesFlavor->addItem(i18n("Decimal degrees"),
                                 static_cast<int>(KGeoTag::DecimalDegrees));
    m_coordinatesFlavor->addItem(i18n("Degrees, decimal minutes"),
                                 static_cast<int>(KGeoTag::DegreesDecimalMinutes));
    m_coordinatesFlavor->addItem(i18n("Degrees, minutes, decimal seconds"),
                                 static_cast<int>(KGeoTag::DegreesMinutesDecimalSeconds));
    m_coordinatesFlavor->setCurrentIndex(
        m_coordinatesFlavor->findData(static_cast<int>(m_settings->coordinatesFlavor())));
    coordinatesSettingsLayout->addWidget(m_coordinatesFlavor, 1, 1);

    auto *coordinatesSettingsLabel = new QLabel(i18n(
        "Changes to the coordinates formatting take effect as soon as the respective coordinates "
        "display is updated the next time. Restart KGeoTag to update everything at once."));
    coordinatesSettingsLabel->setWordWrap(true);
    coordinatesBoxLayout->addWidget(coordinatesSettingsLabel);

    // Image lists

    auto *listsBox = new QGroupBox(i18n("Image lists"));
    layout->addWidget(listsBox);

    auto *listsBoxLayout = new QVBoxLayout(listsBox);

    auto *listsModeLabel = new QLabel(i18n(
        "Loaded images can either be listed in two different lists (one for all images without and "
        "one for images with coordinates), or using a combined consecutive list for all images."));
    listsModeLabel->setWordWrap(true);
    listsBoxLayout->addWidget(listsModeLabel);

    auto *listModeSelectionLayout = new QHBoxLayout;
    listsBoxLayout->addLayout(listModeSelectionLayout);

    listModeSelectionLayout->addWidget(new QLabel(i18n("Use the following images list(s) mode:")));

    m_imageListsMode = new QComboBox;
    m_imageListsMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    m_imageListsMode->addItem(i18n("Separate \"Assigned\" and \"Unassigned\" list"));
    m_imageListsMode->addItem(i18n("One combined list for all images"));

    m_imageListsMode->setCurrentIndex(m_settings->splitImagesList() ? 0 : 1);
    m_originalSplitImagesListValue = m_imageListsMode->currentIndex() == 0;

    listModeSelectionLayout->addWidget(m_imageListsMode);

    listModeSelectionLayout->addStretch();

    // Automatic matching

    auto *searchMatchesBox = new QGroupBox(i18n("Image Assignment"));
    layout->addWidget(searchMatchesBox);

    auto *searchMatchesBoxLayout = new QVBoxLayout(searchMatchesBox);

    auto *searchMatchesSelectionLayout = new QHBoxLayout;
    searchMatchesBoxLayout->addLayout(searchMatchesSelectionLayout);

    searchMatchesSelectionLayout->addWidget(new QLabel(i18n("Search type for \"Correlate images "
                                                            "with GPS data\":")));

    m_automaticMatchingMode = new QComboBox;
    m_automaticMatchingMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);

    m_automaticMatchingMode->addItem(i18n("Combined match search"), KGeoTag::CombinedMatchSearch);
    m_automaticMatchingMode->addItem(i18n("Search exact matches only"), KGeoTag::ExactMatchSearch);
    m_automaticMatchingMode->addItem(i18n("Search interpolated matches only"),
                                     KGeoTag::InterpolatedMatchSearch);

    m_automaticMatchingMode->setCurrentIndex(
        m_automaticMatchingMode->findData(m_settings->defaultMatchingMode()));

    searchMatchesSelectionLayout->addWidget(m_automaticMatchingMode);

    searchMatchesSelectionLayout->addStretch();

    auto *matchModeNoteLabel = new QLabel(i18n(
        "This triggers an automatic (re-)assignment of all images, respecting the \"Exclude "
        "manually tagged images\" setting from the \"Automatic assignment\" dock."));
    matchModeNoteLabel->setWordWrap(true);
    searchMatchesBoxLayout->addWidget(matchModeNoteLabel);

    // Images

    auto *imagesBox = new QGroupBox(i18n("Thumbnails and previews"));
    auto *imagesBoxLayout = new QVBoxLayout(imagesBox);
    layout->addWidget(imagesBox);

    auto *sizesLayoutWrapper = new QHBoxLayout;
    imagesBoxLayout->addLayout(sizesLayoutWrapper);
    auto *sizesLayout = new QGridLayout;
    sizesLayoutWrapper->addLayout(sizesLayout);
    row = -1;

    sizesLayout->addWidget(new QLabel(i18n("Thumbnail size:")), ++row, 0);
    m_thumbnailSize = new QSpinBox;
    m_thumbnailSize->setMinimum(16);
    m_thumbnailSize->setMaximum(512);
    m_originalThumbnailSizeValue = m_settings->thumbnailSize();
    m_thumbnailSize->setValue(m_originalThumbnailSizeValue);
    sizesLayout->addWidget(m_thumbnailSize, row, 1);
    sizesLayout->addWidget(new QLabel(i18n("px")), row, 2);

    sizesLayout->addWidget(new QLabel(i18n("Preview size:")), ++row, 0);
    m_previewSize = new QSpinBox;
    m_previewSize->setMinimum(100);
    m_previewSize->setMaximum(1920);
    m_originalPreviewSizeValue = m_settings->previewSize();
    m_previewSize->setValue(m_originalPreviewSizeValue);
    sizesLayout->addWidget(m_previewSize, row, 1);
    sizesLayout->addWidget(new QLabel(i18n("px")), row, 2);

    sizesLayoutWrapper->addStretch();

    auto *imagesChangesLabel = new QLabel(i18n("Please restart the program after changes to these "
                                               "values so that they are applied and become "
                                               "visible."));
    imagesChangesLabel->setWordWrap(true);
    imagesBoxLayout->addWidget(imagesChangesLabel);

    // GPX track rendering

    auto *trackBox = new QGroupBox(i18n("GPX track rendering"));
    auto *trackBoxLayout = new QHBoxLayout(trackBox);
    layout->addWidget(trackBox);

    auto *renderingLayout = new QGridLayout;
    trackBoxLayout->addLayout(renderingLayout);
    row = -1;

    renderingLayout->addWidget(new QLabel(i18n("Color:")), ++row, 0);
    m_trackColor = new QPushButton;
    m_trackColor->setFlat(true);
    m_currentTrackColor = m_settings->trackColor();
    connect(m_trackColor, &QPushButton::clicked, this, &SettingsDialog::setTrackColor);
    renderingLayout->addWidget(m_trackColor, row, 1);

    m_trackOpacity = new QLabel;
    renderingLayout->addWidget(m_trackOpacity, row, 2);

    updateTrackColor();

    renderingLayout->addWidget(new QLabel(i18n("Line width:")), ++row, 0);
    m_trackWidth = new QSpinBox;
    m_trackWidth->setMinimum(1);
    m_trackWidth->setMaximum(50);
    m_trackWidth->setValue(m_settings->trackWidth());
    renderingLayout->addWidget(m_trackWidth, row, 1);

    renderingLayout->addWidget(new QLabel(i18n("Line style:")), ++row, 0);
    m_trackStyle = new QComboBox;
    m_trackStyle->addItem(i18n("Solid"), static_cast<int>(Qt::SolidLine));
    m_trackStyle->addItem(i18n("Dashes"), static_cast<int>(Qt::DashLine));
    m_trackStyle->addItem(i18n("Dots"), static_cast<int>(Qt::DotLine));
    m_trackStyle->addItem(i18n("Dash-Dot"), static_cast<int>(Qt::DashDotLine));
    m_trackStyle->addItem(i18n("Dash-Dot-Dot"), static_cast<int>(Qt::DashDotDotLine));
    m_trackStyle->setCurrentIndex(
        m_trackStyle->findData(static_cast<int>(m_settings->trackStyle())));
    renderingLayout->addWidget(m_trackStyle, row, 1);

    trackBoxLayout->addStretch();

    // Elevation lookup

    auto *elevationBox = new QGroupBox(i18n("Elevation lookup"));
    auto *elevationBoxLayout = new QVBoxLayout(elevationBox);
    layout->addWidget(elevationBox);

    auto *lookupLabel = new QLabel(i18n("Elevations can be looked up using opentopodata.org's web "
                                        "API."));
    lookupLabel->setWordWrap(true);
    elevationBoxLayout->addWidget(lookupLabel);

    auto *datasetLayout = new QHBoxLayout;
    elevationBoxLayout->addLayout(datasetLayout);

    datasetLayout->addWidget(new QLabel(i18n("Elevation dataset:")));

    m_elevationDataset = new QComboBox;

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

    m_elevationDataset->setCurrentIndex(
        m_elevationDataset->findData(m_settings->elevationDataset()));

    datasetLayout->addWidget(m_elevationDataset);

    datasetLayout->addStretch();

    auto *datasetInfoLabel = new QLabel(i18n("Cf. <a href=\"https://www.opentopodata.org/\">"
                                             "https://www.opentopodata.org/</a> for further "
                                             "information about the available datasets, like the "
                                             "respective coverage!"));
    datasetInfoLabel->setWordWrap(true);
    datasetInfoLabel->setOpenExternalLinks(true);
    elevationBoxLayout->addWidget(datasetInfoLabel);

    m_lookupElevationAutomatically = new QCheckBox(i18n("Request and set altitudes automatically"));
    m_lookupElevationAutomatically->setChecked(m_settings->lookupElevationAutomatically());
    elevationBoxLayout->addWidget(m_lookupElevationAutomatically);

    // Data saving

    auto *saveBox = new QGroupBox(i18n("Saving"));
    auto *saveBoxLayout = new QVBoxLayout(saveBox);
    layout->addWidget(saveBox);

    auto *saveTargetLayout = new QHBoxLayout;
    saveBoxLayout->addLayout(saveTargetLayout);

    saveTargetLayout->addWidget(new QLabel(i18n("Write changes to:")));

    m_writeMode = new QComboBox;

    m_writeMode->addItem(i18n("Exif header"),
                         QStringLiteral("WRITETOIMAGEONLY"));
    m_writeMode->addItem(i18n("XMP sidecar file"),
                         QStringLiteral("WRITETOSIDECARONLY"));
    m_writeMode->addItem(i18n("Exif header and XMP sidecar file"),
                         QStringLiteral("WRITETOSIDECARANDIMAGE"));

    m_writeMode->setCurrentIndex(m_writeMode->findData(m_settings->writeMode()));

    saveTargetLayout->addWidget(m_writeMode);

    saveTargetLayout->addStretch();

    m_allowWriteRawFiles = new QCheckBox(i18n("Allow altering Exif headers of RAW images"));
    m_allowWriteRawFiles->setChecked(m_settings->allowWriteRawFiles());
    saveBoxLayout->addWidget(m_allowWriteRawFiles);

    auto *label = new QLabel(i18n(
        "<b>Caution!</b> This is experimental. By default, regardless of the setting for \"Write "
        "changes to\", all changes to RAW images are written to XMP sidecar files. Have backups "
        "and check your Exif data to be still complete and correct when using this!"
    ));
    label->setWordWrap(true);
    label->setIndent(30);
    saveBoxLayout->addWidget(label);

    m_createBackups = new QCheckBox(i18n("Create a backup before altering a file"));
    m_createBackups->setChecked(m_settings->createBackups());
    saveBoxLayout->addWidget(m_createBackups);

    // Scroll area

    auto *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(settingsWidget);
    mainLayout->addWidget(scrollArea);

    updateGeometry();

    scrollArea->setMinimumWidth(
        settingsWidget->width()
        + qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent)
        + qApp->style()->pixelMetric(QStyle::PM_DefaultFrameWidth)
    );

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

void SettingsDialog::accept()
{
    m_settings->saveLatBeforeLon(m_coordinatesOrder->currentIndex());
    m_settings->saveCoordinatesFlavor(static_cast<KGeoTag::CoordinatesFlavor>(
        m_coordinatesFlavor->currentData().toInt()));

    const auto splitImagesList = m_imageListsMode->currentIndex() == 0;
    m_settings->saveSplitImagesList(splitImagesList);

    m_settings->saveDefaultMatchingMode(static_cast<KGeoTag::SearchType>(
        m_automaticMatchingMode->currentData().toInt()));

    const auto thumbnailSize = m_thumbnailSize->value();
    m_settings->saveThumbnailSize(thumbnailSize);
    const auto previewSize = m_previewSize->value();
    m_settings->savePreviewSize(previewSize);

    m_settings->saveTrackColor(m_currentTrackColor);
    m_settings->saveTrackWidth(m_trackWidth->value());
    m_settings->saveTrackStyle(static_cast<Qt::PenStyle>(m_trackStyle->currentData().toInt()));

    m_settings->saveLookupElevationAutomatically(m_lookupElevationAutomatically->isChecked());
    m_settings->saveElevationDataset(m_elevationDataset->currentData().toString());

    m_settings->saveWriteMode(m_writeMode->currentData().toString());
    m_settings->saveAllowWriteRawFiles(m_allowWriteRawFiles->isChecked());
    m_settings->saveCreateBackups(m_createBackups->isChecked());

    if (   thumbnailSize != m_originalThumbnailSizeValue
        || previewSize != m_originalPreviewSizeValue) {

        QMessageBox::information(this, i18n("Settings changed"),
            i18n("Please restart KGeoTag to apply the changed settings and make them visible!"));
    }

    if (splitImagesList != m_originalSplitImagesListValue) {
        Q_EMIT imagesListsModeChanged();
    }

    QDialog::accept();
}
