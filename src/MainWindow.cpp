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
#include "MainWindow.h"
#include "Settings.h"
#include "ImageCache.h"
#include "GpxEngine.h"
#include "ElevationEngine.h"
#include "PreviewWidget.h"
#include "MapWidget.h"
#include "KGeoTag.h"
#include "ImagesList.h"
#include "SettingsDialog.h"
#include "FixDriftWidget.h"

// KDE includes
#include <KLocalizedString>
#include <KIconLoader>
#include <KHelpMenu>
#include <KExiv2/KExiv2>

// Qt includes
#include <QMenuBar>
#include <QAction>
#include <QDockWidget>
#include <QGuiApplication>
#include <QScreen>
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QProgressDialog>
#include <QFile>
#include <QTimer>
#include <QMessageBox>
#include <QCloseEvent>

// C++ includes
#include <algorithm>

MainWindow::MainWindow() : QMainWindow()
{
    setWindowTitle(i18n("KGeoTag"));

    m_settings = new Settings(this);
    m_imageCache = new ImageCache(this, m_settings);
    m_gpxEngine = new GpxEngine(this, m_settings);
    m_elevationEngine = new ElevationEngine(this);
    connect(m_elevationEngine, &ElevationEngine::elevationProcessed,
            this, &MainWindow::elevationProcessed);

    // Menu setup

    // File

    auto *fileMenu = menuBar()->addMenu(i18n("File"));

    auto *addGpxAction = fileMenu->addAction(i18n("Add GPX track"));
    connect(addGpxAction, &QAction::triggered, this, &MainWindow::addGpx);

    auto *addImagesAction = fileMenu->addAction(i18n("Add images"));
    connect(addImagesAction, &QAction::triggered, this, &MainWindow::addImages);

    fileMenu->addSeparator();

    auto *saveChangesAction = fileMenu->addAction(i18n("Save changed images"));
    connect(saveChangesAction, &QAction::triggered, this, &MainWindow::saveChanges);

    fileMenu->addSeparator();

    auto *quitAction = fileMenu->addAction(i18n("Quit"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // Edit

    auto *editMenu = menuBar()->addMenu(i18n("Edit"));

    auto *assignExactMatchesAction = editMenu->addAction(i18n("Assign images (exact matches)"));
    connect(assignExactMatchesAction, &QAction::triggered, this, &MainWindow::assignExactMatches);

    auto *assignInterpolatedMatchesAction = editMenu->addAction(i18n("Assign images "
                                                                     "(interpolated)"));
    connect(assignInterpolatedMatchesAction, &QAction::triggered,
            this, &MainWindow::assignInterpolatedMatches);

    editMenu->addSeparator();

    auto *removeAllCoordinatesAction = editMenu->addAction(i18n("Remove coordinates from all "
                                                                "images"));
    connect(removeAllCoordinatesAction, &QAction::triggered,
            this, &MainWindow::removeAllCoordinates);

    editMenu->addSeparator();

    auto *discardAllChangesAction = editMenu->addAction(i18n("Discard all changes"));
    connect(discardAllChangesAction, &QAction::triggered, this, &MainWindow::discardAllChanges);

    editMenu->addSeparator();

    auto *showSettingsAction = editMenu->addAction(i18n("Settings"));
    connect(showSettingsAction, &QAction::triggered, this, &MainWindow::showSettings);

    // Help
    auto *helpMenu = new KHelpMenu;
    menuBar()->addMenu(helpMenu->menu());

    // Dock setup
    setDockNestingEnabled(true);

    // Unassigned images
    m_unAssignedImages = new ImagesList(ImagesList::Type::UnAssigned, m_settings, m_imageCache);
    auto *unassignedImagesDock = createDockWidget(i18n("Unassigned images"), m_unAssignedImages,
                                                  QStringLiteral("unassignedImagesDock"));
    connect(m_unAssignedImages, &ImagesList::removeCoordinates,
            this, &MainWindow::removeCoordinates);
    connect(m_unAssignedImages, &ImagesList::discardChanges, this, &MainWindow::discardChanges);

    // Assigned images
    m_assignedImages = new ImagesList(ImagesList::Type::Assigned, m_settings, m_imageCache);
    auto *assignedImagesDock = createDockWidget(i18n("Assigned images"), m_assignedImages,
                                                QStringLiteral("assignedImagesDock"));
    connect(m_assignedImages, &ImagesList::lookupElevation, this, &MainWindow::lookupElevation);
    connect(m_assignedImages, &ImagesList::removeCoordinates, this, &MainWindow::removeCoordinates);
    connect(m_assignedImages, &ImagesList::discardChanges, this, &MainWindow::discardChanges);

    // Preview
    m_previewWidget = new PreviewWidget(m_imageCache);
    auto *previewDock = createDockWidget(i18n("Preview"), m_previewWidget,
                                         QStringLiteral("previewDock"));
    connect(m_assignedImages, &ImagesList::imageSelected,
            m_previewWidget, &PreviewWidget::setImage);
    connect(m_unAssignedImages, &ImagesList::imageSelected,
            m_previewWidget, &PreviewWidget::setImage);

    // Fix drift
    m_fixDriftWidget = new FixDriftWidget;
    auto *fixDriftDock = createDockWidget(i18n("Fix time drift"), m_fixDriftWidget,
                                          QStringLiteral("fixDriftDock"));

    // Map
    m_mapWidget = new MapWidget(m_settings, m_imageCache);
    createDockWidget(i18n("Map"), m_mapWidget, QStringLiteral("mapDock"));
    connect(m_gpxEngine, &GpxEngine::segmentLoaded, m_mapWidget, &MapWidget::addSegment);
    connect(m_assignedImages, &ImagesList::imageSelected, this, &MainWindow::imageSelected);
    connect(m_mapWidget, &MapWidget::imageDropped, this, &MainWindow::imageDropped);

    // Size initialization/restoration
    if (! restoreGeometry(m_settings->mainWindowGeometry())) {
        const QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();
        if (availableGeometry.width() > 1024 && availableGeometry.height() > 765) {
            resize(QSize(1024, 765));
        } else {
            setWindowState(Qt::WindowMaximized);
        }
    }

    // Initialize/Restore the dock widget arrangement
    if (! restoreState(m_settings->mainWindowState())) {
        splitDockWidget(assignedImagesDock, previewDock, Qt::Vertical);
        splitDockWidget(assignedImagesDock, unassignedImagesDock, Qt::Horizontal);
        splitDockWidget(previewDock, fixDriftDock, Qt::Vertical);
        tabifyDockWidget(previewDock, fixDriftDock);
        previewDock->raise();
    }

    // Restore the map's settings
    m_mapWidget->restoreSettings();
}

QDockWidget *MainWindow::createDockWidget(const QString &title, QWidget *widget,
                                          const QString &objectName)
{
    auto *dock = new QDockWidget(title, this);
    dock->setObjectName(objectName);
    dock->setContextMenuPolicy(Qt::PreventContextMenu);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock->setWidget(widget);
    addDockWidget(Qt::TopDockWidgetArea, dock);
    return dock;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (! m_imageCache->changedImages().isEmpty()) {
        if (QMessageBox::question(this, i18n("Close KGeoTag"),
            i18n("<p>There are pending changes to images that haven't been saved yet. All changes "
                 "will be discarded if KGeoTag is closed now.</p>"
                 "<p>Do you want to close the program anyway?</p>"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {

            event->ignore();
            return;
        }
    }

    m_settings->saveMainWindowGeometry(saveGeometry());
    m_settings->saveMainWindowState(saveState());

    m_mapWidget->saveSettings();

    QApplication::quit();
}

void MainWindow::addGpx()
{
    const auto path = QFileDialog::getOpenFileName(this,
                          i18n("Please select the GPX track to add"),
                          m_settings->lastOpenPath(),
                          i18n("GPX tracks (*.gpx)"));
    if (path.isEmpty()) {
        return;
    }

    const QFileInfo info(path);
    m_settings->saveLastOpenPath(info.dir().absolutePath());

    QApplication::setOverrideCursor(Qt::WaitCursor);
    const auto [ result, points, segments ] = m_gpxEngine->load(path);
    m_mapWidget->zoomToGpxBox();
    QApplication::restoreOverrideCursor();

    switch (result) {
    case GpxEngine::LoadResult::Okay:
        QMessageBox::information(this, i18n("Add GPX file"),
            i18nc("E. g. \"Successfully loaded 18,000 waypoints from 8 track segments\", the "
                  "plural forms are compiled in the following two i18np calls.",
                  "<p>Successfully loaded %1 from %2!</p>",
                 i18np("1 waypoint", "%1 waypoints", points),
                 i18np("1 track segment", "%1 track segments", segments)));
        break;
    case GpxEngine::LoadResult::OpenFailed:
        QMessageBox::warning(this, i18n("Add GPX file"),
            i18n("<p>Opening <kbd>%1</kbd> failed. Please be sure to have read access to this "
                 "file.</p>", path));
        break;
    case GpxEngine::LoadResult::XmlError:
        QMessageBox::warning(this, i18n("Add GPX file"),
            i18n("<p>XML parsing failed for <kbd>%1</kbd>. Either, no data could be loaded at "
                 "all, or only a part of it.</p>",
                 path));
        break;
    }
}

void MainWindow::addImages()
{
    const auto files = QFileDialog::getOpenFileNames(this,
                           i18n("Please select the images to add"),
                           m_settings->lastOpenPath(),
                           i18n("JPEG Images (*.jpg *.jpeg)"));
    if (files.isEmpty()) {
        return;
    }

    const QFileInfo info(files.at(0));
    m_settings->saveLastOpenPath(info.dir().absolutePath());

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QProgressDialog progress(i18n("Loading images ..."), i18n("Cancel"), 0, files.count(), this);
    progress.setWindowModality(Qt::WindowModal);

    int processed = 0;
    for (const auto &path : files) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        const QFileInfo info(path);
        const QString canonicalPath = info.canonicalFilePath();
        if (! m_imageCache->addImage(canonicalPath)) {
            continue;
        }

        const auto coordinates = m_imageCache->coordinates(canonicalPath);
        if (! coordinates.isSet) {
            m_unAssignedImages->addOrUpdateImage(canonicalPath);
        } else {
            m_mapWidget->addImage(canonicalPath, coordinates.lon, coordinates.lat);
            m_assignedImages->addOrUpdateImage(canonicalPath);
        }
    }

    QApplication::restoreOverrideCursor();
}

void MainWindow::imageDropped(const QString &path)
{
    m_unAssignedImages->removeImage(path);
    m_imageCache->setMatchType(path, KGeoTag::MatchType::Set);
    m_imageCache->setChanged(path, true);
    m_assignedImages->addOrUpdateImage(path);
    m_previewWidget->setImage(path);

    if (m_settings->lookupElevation()) {
        lookupElevation(path);
    }
}

void MainWindow::lookupElevation(const QString &path)
{
    QApplication::setOverrideCursor(Qt::BusyCursor);
    m_elevationEngine->request(path, m_imageCache->coordinates(path));
}

void MainWindow::assignImage(const QString &path, const KGeoTag::Coordinates &coordinates)
{
    m_imageCache->setCoordinates(path, coordinates);
    m_unAssignedImages->removeImage(path);
    m_mapWidget->addImage(path, coordinates);
    m_assignedImages->addOrUpdateImage(path);
}

void MainWindow::assignExactMatches()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    const int deviation = m_fixDriftWidget->deviation();
    const auto images = m_unAssignedImages->allImages();
    for (const auto &path : images) {
        const auto coordinates = m_gpxEngine->findExactCoordinates(m_imageCache->date(path),
                                                                   deviation);
        if (coordinates.isSet) {
            m_imageCache->setMatchType(path, KGeoTag::MatchType::Exact);
            m_imageCache->setChanged(path, true);
            assignImage(path, coordinates);
        }
    }

    m_mapWidget->reloadMap();

    QApplication::restoreOverrideCursor();
}

void MainWindow::assignInterpolatedMatches()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    const int deviation = m_fixDriftWidget->deviation();
    const auto files = m_unAssignedImages->allImages();

    QProgressDialog progress(i18n("Assigning images ..."), i18n("Cancel"), 0, files.count(), this);
    progress.setWindowModality(Qt::WindowModal);

    int processed = 0;
    for (const auto &path : files) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        const auto coordinates = m_gpxEngine->findInterpolatedCoordinates(m_imageCache->date(path),
                                                                          deviation);
        if (coordinates.isSet) {
            m_imageCache->setMatchType(path, KGeoTag::MatchType::Interpolated);
            m_imageCache->setChanged(path, true);
            assignImage(path, coordinates);
        }
    }

    m_mapWidget->reloadMap();

    QApplication::restoreOverrideCursor();
}

void MainWindow::saveChanges()
{
    auto files = m_imageCache->changedImages();

    if (files.isEmpty()) {
        QMessageBox::information(this, i18n("Save changes"), i18n("Nothing to do"));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // We sort the list so that the processing order matches the display order
    std::sort(files.begin(), files.end());

    QProgressDialog progress(i18n("Saving changes ..."), i18n("Cancel"), 0, files.count(), this);
    progress.setWindowModality(Qt::WindowModal);

    const bool createBackups = m_settings->createBackups();
    const int deviation = m_fixDriftWidget->deviation();
    const bool fixDrift = m_fixDriftWidget->save() && deviation != 0;

    int processed = 0;
    for (const QString &path : files) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        // Create a backup of the file if requested

        const QString backupPath = path + QStringLiteral(".orig");

        if (createBackups && ! QFile::copy(path, backupPath)) {
            progress.deleteLater();
            QApplication::restoreOverrideCursor();

            QTimer::singleShot(0, [this, path, backupPath]
            {
                QFileInfo info(backupPath);

                QMessageBox::warning(this,
                    i18n("Save changes"),
                    i18n("<p><b>Saving changes failed</b></p>"
                         "<p>Could not save changes to <kbd>%1</kbd>: The backup file "
                         "<kbd>%2</kbd> could not be created.</p>"
                         "<p>Please check if this file doesn't exist yet and be sure to have "
                         "write access to <kbd>%3</kbd></p>"
                         "<p>The writing process will be aborted.</p>",
                         path, info.fileName(), info.dir().path()));
            });

            return;
        }

        // Write the GPS information

        auto exif = KExiv2Iface::KExiv2(path);

        // Set or remove the coordinates
        const auto coordinates = m_imageCache->coordinates(path);
        if (coordinates.isSet) {
            exif.setGPSInfo(0.0, coordinates.lat, coordinates.lon);
        } else {
            exif.removeGPSInfo();
        }

        // Fix the time drift if requested
        if (fixDrift) {
            const QDateTime originalTime = m_imageCache->date(path);
            const QDateTime fixedTime = originalTime.addSecs(deviation);
            // If the Digitization time is equal to the original time, update it as well.
            // Otherwise, only update the image's timestamp.
            exif.setImageDateTime(fixedTime, exif.getDigitizationDateTime() == originalTime);
        }

        if (! exif.save(path)) {
            progress.deleteLater();
            QApplication::restoreOverrideCursor();

            QTimer::singleShot(0, [this, path]
            {
                QMessageBox::warning(this,
                    i18n("Save changes"),
                    i18n("<p><b>Saving changes failed</b></p>"
                         "<p>Could not write Exif header to <kbd>%1</kbd>. Please check the file's "
                         "permissions and be sure to have write access.</p>"
                         "<p>The writing process will be aborted.</p>", path));
            });

            return;
        }

        m_imageCache->setChanged(path, false);
        if (coordinates.isSet) {
            m_assignedImages->addOrUpdateImage(path);
        } else {
            m_unAssignedImages->addOrUpdateImage(path);
        }
    }

    QApplication::restoreOverrideCursor();
    QMessageBox::information(this, i18n("Save changes"),
                             i18n("All changes have been successfully saved!"));
}

void MainWindow::imageSelected(const QString &path, bool center)
{
    if (center) {
        m_mapWidget->centerImage(path);
    }
}

void MainWindow::showSettings()
{
    SettingsDialog dialog(m_settings, this);
    if (! dialog.exec()) {
        return;
    }

    m_mapWidget->updateSettings();
}

void MainWindow::removeCoordinates(const QString &path)
{
    m_imageCache->setCoordinates(path, KGeoTag::NoCoordinates);
    m_imageCache->setChanged(path, true);
    m_imageCache->setMatchType(path, KGeoTag::MatchType::None);
    m_assignedImages->removeImage(path);
    m_unAssignedImages->addOrUpdateImage(path);
    m_mapWidget->removeImage(path);
    m_mapWidget->reloadMap();
    m_previewWidget->setImage(path);
}

void MainWindow::discardChanges(const QString &path)
{
    m_imageCache->resetChanges(path);
    const auto coordinates = m_imageCache->coordinates(path);
    if (coordinates.isSet) {
        m_unAssignedImages->removeImage(path);
        m_assignedImages->addOrUpdateImage(path);
        m_mapWidget->addImage(path, coordinates);
    } else {
        m_assignedImages->removeImage(path);
        m_unAssignedImages->addOrUpdateImage(path);
        m_mapWidget->removeImage(path);
    }
    m_mapWidget->reloadMap();
    m_previewWidget->setImage(path);
}

void MainWindow::discardAllChanges()
{
    const auto files = m_imageCache->changedImages();
    for (const auto &path : files) {
        m_imageCache->resetChanges(path);
        m_assignedImages->removeImage(path);
        m_unAssignedImages->removeImage(path);
        m_mapWidget->removeImage(path);

        const auto coordinates = m_imageCache->coordinates(path);
        if (coordinates.isSet) {
            m_assignedImages->addOrUpdateImage(path);
            m_mapWidget->addImage(path, coordinates);
        } else {
            m_unAssignedImages->addOrUpdateImage(path);
        }
    }

    m_mapWidget->reloadMap();
    m_previewWidget->setImage(QString());
}

void MainWindow::removeAllCoordinates()
{
    if (QMessageBox::question(this, i18n("Remove coordinates from all images"),
            i18n("Do you really want to remove all set coordinates from all images?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {

        return;
    }

    const auto images = m_assignedImages->allImages();
    for (const auto &path : images) {
        m_imageCache->setCoordinates(path, KGeoTag::NoCoordinates);
        m_imageCache->setChanged(path, true);
        m_imageCache->setMatchType(path, KGeoTag::MatchType::None);
        m_assignedImages->removeImage(path);
        m_unAssignedImages->addOrUpdateImage(path);
        m_mapWidget->removeImage(path);
    }
    m_mapWidget->reloadMap();
}

void MainWindow::elevationProcessed(QString path, bool success, double elevation)
{
    QApplication::restoreOverrideCursor();
    if (! success) {
        if (QMessageBox::warning(this, i18n("Elevation lookup"),
            i18n("<p>Fetching elevation data from opentopodata.org failed.</p>"
                 "<p>Should the elevation lookup be disabled?</p>"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {

            m_settings->saveLookupElevation(false);
        }
        return;
    }

    auto coordinates = m_imageCache->coordinates(path);
    coordinates.alt = elevation;
    m_imageCache->setCoordinates(path, coordinates);

    if (m_previewWidget->currentImage() == path) {
        m_previewWidget->setImage(path);
    }
}
