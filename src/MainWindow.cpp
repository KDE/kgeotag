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
#include "SharedObjects.h"
#include "Settings.h"
#include "GpxEngine.h"
#include "PreviewWidget.h"
#include "MapWidget.h"
#include "KGeoTag.h"
#include "SettingsDialog.h"
#include "FixDriftWidget.h"
#include "BookmarksList.h"
#include "ElevationEngine.h"
#include "BookmarksWidget.h"
#include "CoordinatesDialog.h"
#include "RetrySkipAbortDialog.h"
#include "ImagesModel.h"
#include "ImagesListView.h"

// KDE includes
#include <KLocalizedString>
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
#include <QAbstractButton>

// C++ includes
#include <algorithm>

static const QHash<QString, KExiv2Iface::KExiv2::MetadataWritingMode> s_writeModeMap {
    { QStringLiteral("WRITETOIMAGEONLY"),
      KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOIMAGEONLY },
    { QStringLiteral("WRITETOSIDECARONLY"),
      KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOSIDECARONLY },
    { QStringLiteral("WRITETOSIDECARANDIMAGE"),
      KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOSIDECARANDIMAGE }
};

MainWindow::MainWindow(SharedObjects *sharedObjects)
    : QMainWindow(),
      m_sharedObjects(sharedObjects),
      m_settings(sharedObjects->settings()),
      m_gpxEngine(sharedObjects->gpxEngine()),
      m_elevationEngine(sharedObjects->elevationEngine()),
      m_imagesModel(sharedObjects->imagesModel())
{
    setWindowTitle(i18n("KGeoTag"));

    connect(m_elevationEngine, &ElevationEngine::elevationProcessed,
            this, &MainWindow::elevationProcessed);

    // Menu setup
    // ==========

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

    // Settings
    auto *settingsMenu = menuBar()->addMenu(i18n("Settings"));

    auto *setDefaultDockArrangementAction = settingsMenu->addAction(i18n("Set default dock "
                                                                         "arrangement"));
    connect(setDefaultDockArrangementAction, &QAction::triggered,
            this, &MainWindow::setDefaultDockArrangement);

    settingsMenu->addSeparator();

    auto *showSettingsAction = settingsMenu->addAction(i18n("Configure KGeoTag"));
    connect(showSettingsAction, &QAction::triggered, this, &MainWindow::showSettings);

    // Help
    auto *helpMenu = new KHelpMenu;
    menuBar()->addMenu(helpMenu->menu());

    // Dock setup
    // ==========

    setDockNestingEnabled(true);

    // Bookmarks
    m_bookmarksWidget = new BookmarksWidget(m_sharedObjects);
    m_sharedObjects->setBookmarks(m_bookmarksWidget->bookmarks());
    m_bookmarksDock = createDockWidget(i18n("Bookmarks"), m_bookmarksWidget,
                                           QStringLiteral("bookmarksDock"));

    // Preview
    m_previewWidget = new PreviewWidget(m_sharedObjects);
    m_previewDock = createDockWidget(i18n("Preview"), m_previewWidget,
                                         QStringLiteral("previewDock"));

    // Fix drift
    m_fixDriftWidget = new FixDriftWidget;
    m_fixDriftDock = createDockWidget(i18n("Fix time drift"), m_fixDriftWidget,
                                          QStringLiteral("fixDriftDock"));

    // Map
    m_mapWidget = m_sharedObjects->mapWidget();
    m_mapDock = createDockWidget(i18n("Map"), m_mapWidget, QStringLiteral("mapDock"));
    connect(m_gpxEngine, &GpxEngine::segmentLoaded, m_mapWidget, &MapWidget::addSegment);
    connect(m_mapWidget, &MapWidget::imagesDropped, this, &MainWindow::imagesDropped);

    // Images lists

    m_unAssignedImagesDock = createImagesDock(KGeoTag::ImagesListType::UnAssigned,
                                              i18n("Unassigned images"),
                                              QStringLiteral("unAssignedImagesDock"));
    if (m_settings->splitImagesList()) {
        m_assignedOrAllImagesDock = createImagesDock(KGeoTag::ImagesListType::Assigned,
                                                     i18n("Assigned images"),
                                                     QStringLiteral("assignedOrAllImagesDock"));
    } else {
        m_assignedOrAllImagesDock = createImagesDock(KGeoTag::ImagesListType::All,
                                                     i18n("Images"),
                                                     QStringLiteral("assignedOrAllImagesDock"));
    }

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
        setDefaultDockArrangement();
    } else {
        m_unAssignedImagesDock->setVisible(m_settings->splitImagesList());
    }

    // Restore the map's settings
    m_mapWidget->restoreSettings();

    // Handle failed elevation lookups and missing locations
    connect(m_sharedObjects->elevationEngine(), &ElevationEngine::lookupFailed,
            this, &MainWindow::elevationLookupFailed);
    connect(m_sharedObjects->elevationEngine(), &ElevationEngine::notAllPresent,
            this, &MainWindow::notAllElevationsPresent);
}

QDockWidget *MainWindow::createImagesDock(KGeoTag::ImagesListType type, const QString &title,
                                          const QString &dockId)
{
    auto *list = new ImagesListView(type, m_sharedObjects);

    connect(list, &ImagesListView::imageSelected, m_previewWidget, &PreviewWidget::setImage);
    connect(list, &ImagesListView::centerImage, m_mapWidget, &MapWidget::centerImage);
    connect(m_bookmarksWidget, &BookmarksWidget::bookmarksChanged,
            list, &ImagesListView::updateBookmarks);
    connect(list, &ImagesListView::searchExactMatches, this, &MainWindow::searchExactMatches);
    connect(list, &ImagesListView::searchInterpolatedMatches,
            this, &MainWindow::searchInterpolatedMatches);
    connect(list, &ImagesListView::assignToMapCenter, this, &MainWindow::assignToMapCenter);
    connect(list, &ImagesListView::assignManually, this, &MainWindow::assignManually);
    connect(list, &ImagesListView::editCoordinates, this, &MainWindow::editCoordinates);
    connect(list, &ImagesListView::removeCoordinates, this, &MainWindow::removeCoordinates);
    connect(list, &ImagesListView::discardChanges, this, &MainWindow::discardChanges);
    connect(list, &ImagesListView::lookupElevation,
            this, QOverload<ImagesListView *>::of(&MainWindow::lookupElevation));
    connect(list, &ImagesListView::assignTo, this, &MainWindow::assignTo);

    return createDockWidget(title, list, dockId);
}

void MainWindow::setDefaultDockArrangement()
{
    const QVector<QDockWidget *> allDocks = {
        m_assignedOrAllImagesDock,
        m_unAssignedImagesDock,
        m_mapDock,
        m_previewDock,
        m_fixDriftDock,
        m_bookmarksDock
    };

    for (auto *dock : allDocks) {
        dock->setFloating(false);
        addDockWidget(Qt::TopDockWidgetArea, dock);
    }

    for (int i = 1; i < allDocks.count(); i++) {
        splitDockWidget(allDocks.at(i - 1), allDocks.at(i), Qt::Horizontal);
    }

    splitDockWidget(m_assignedOrAllImagesDock, m_previewDock, Qt::Vertical);
    splitDockWidget(m_assignedOrAllImagesDock, m_unAssignedImagesDock, Qt::Horizontal);

    tabifyDockWidget(m_previewDock, m_fixDriftDock);
    tabifyDockWidget(m_fixDriftDock, m_bookmarksDock);
    m_previewDock->raise();

    const double windowWidth = double(width());
    resizeDocks({ m_previewDock, m_mapDock },
                { int(windowWidth * 0.4), int(windowWidth * 0.6) },
                Qt::Horizontal);
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
    if (! m_imagesModel->changedImages().isEmpty()) {
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

    m_settings->saveBookmarks(m_bookmarksWidget->bookmarks());

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
                           i18n("JPEG Images (*.jpg *.jpeg);; All files (*)"));
    if (files.isEmpty()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    const QFileInfo info(files.at(0));
    m_settings->saveLastOpenPath(info.dir().absolutePath());

    const int allImages = files.count();
    const bool isSingleFile = allImages == 1;
    int processed = 0;
    int loaded = 0;
    bool skipImage = false;
    bool abortLoad = false;

    QProgressDialog progress(i18n("Loading images ..."), i18n("Cancel"), 0, allImages, this);
    progress.setWindowModality(Qt::WindowModal);

    for (const auto &path : files) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        const QFileInfo info(path);
        const QString canonicalPath = info.canonicalFilePath();
        while (! m_imagesModel->addImage(canonicalPath)) {
            progress.reset();
            QApplication::restoreOverrideCursor();

            RetrySkipAbortDialog dialog(this,
                i18n("Add images"),
                i18n("<p><b>Loading image failed%1</b></p>"
                     "<p>Could not read <kbd>%2</kbd>.</p>"
                     "<p>Please check if this file is actually a supported image (JPEG, PNG) and "
                     "if you have read access to it.</p>"
                     "<p>%3</p>",
                        isSingleFile ? QString()
                                    : i18nc("Fraction of processed files", " (%1 of %2)",
                                            processed, allImages),
                        path,
                        isSingleFile ? i18n("You can retry to load this file or cancel the loading "
                                            "process.")
                                     : i18n("You can retry to load this file, skip it or cancel "
                                            "the loading process.")),
                        isSingleFile);

            const auto reply = dialog.exec();
            if (reply == RetrySkipAbortDialog::Skip) {
                skipImage = true;
                break;
            } else if (reply == RetrySkipAbortDialog::Abort) {
                abortLoad = true;
                break;
            }

            QApplication::setOverrideCursor(Qt::WaitCursor);
        }

        if (skipImage) {
            skipImage = false;
            continue;
        }
        if (abortLoad) {
            break;
        }

        const auto coordinates = m_imagesModel->coordinates(canonicalPath);
        if (coordinates.isSet) {
            m_mapWidget->addImage(canonicalPath, coordinates.lon, coordinates.lat);
        }

        loaded++;
    }

    progress.reset();
    QApplication::restoreOverrideCursor();

    if (loaded == 0) {
        QMessageBox::warning(this, i18n("Add images"),
                             i18n("Could not load any images!"));
    } else if (loaded < allImages) {
        QMessageBox::warning(this, i18n("Add images"),
            i18np("<p>Could not load all images!</p><p>Successfully loaded one image of %2, %3</p>",
                  "<p>Could not load all images!</p><p>Successfully loaded %1 images of %2, %3</p>",
                  // Not necessary with wording in English, but for other languages
                  loaded, allImages, i18np("one failed to load.",
                                           "%1 failed to load.",
                                           allImages - loaded)));
    } else {
        QMessageBox::information(this, i18n("Add images"),
                                 i18np("Successfully loaded one image!",
                                       "Successfully loaded %1 images!",
                                       allImages));
    }
}

void MainWindow::imagesDropped(const QVector<QString> &paths)
{
    for (const auto &path : paths) {
        m_imagesModel->setMatchType(path, KGeoTag::MatchType::Set);
        m_imagesModel->setChanged(path, true);
    }

    m_previewWidget->setImage(paths.last());

    if (m_settings->lookupElevation()) {
        lookupElevation(paths);
    }
}

void MainWindow::assignToMapCenter(ImagesListView *list)
{
    assignTo(list->selectedPaths(), m_mapWidget->currentCenter());
}

void MainWindow::assignManually(ImagesListView *list)
{
    const auto paths = list->selectedPaths();

    QString label;
    if (paths.count() == 1) {
        QFileInfo info(paths.first());
        label = i18nc("A quoted filename", "\"%1\"", info.fileName());
    } else {
        // We don't need this for English, but possibly for languages with other plural forms
        label = i18np("1 image", "%1 images", paths.count());
    }

    CoordinatesDialog dialog(CoordinatesDialog::Mode::EditCoordinates,
                             m_settings->lookupElevation(), KGeoTag::NoCoordinates, label);
    if (! dialog.exec()) {
        return;
    }

    assignTo(paths, dialog.coordinates());
}

void MainWindow::editCoordinates(ImagesListView *list)
{
    const auto paths = list->selectedPaths();
    auto coordinates = m_imagesModel->coordinates(paths.first());
    bool identicalCoordinates = true;
    for (int i = 1; i < paths.count(); i++) {
        if (m_imagesModel->coordinates(paths.at(i)) != coordinates) {
            identicalCoordinates = false;
            break;
        }
    }

    QString label;
    if (paths.count() == 1) {
        QFileInfo info(paths.first());
        label = i18nc("A quoted filename", "\"%1\"", info.fileName());
    } else {
        // We don't need this for English, but possibly for languages with other plural forms
        label = i18np("1 image (%2)", "%1 images (%2)", paths.count(),
                      identicalCoordinates ? i18n("all images have the same coordinates")
                                           : i18n("coordinates differ across the images"));
    }

    CoordinatesDialog dialog(CoordinatesDialog::Mode::EditCoordinates, false,
                             identicalCoordinates ? coordinates : KGeoTag::NoCoordinates,
                             label);
    if (! dialog.exec()) {
        return;
    }

    assignTo(paths, dialog.coordinates());
}

void MainWindow::assignTo(const QVector<QString> &paths, const KGeoTag::Coordinates &coordinates)
{
    for (const auto &path : paths) {
        m_imagesModel->setMatchType(path, KGeoTag::MatchType::Set);
        assignImage(path, coordinates);
        m_mapWidget->addImage(path, coordinates.lon, coordinates.lat);
    }

    m_mapWidget->centerCoordinates(coordinates);
    m_mapWidget->reloadMap();

    if (m_settings->lookupElevation()) {
        lookupElevation(paths);
    }
}

void MainWindow::assignImage(const QString &path, const KGeoTag::Coordinates &coordinates)
{
    m_imagesModel->setCoordinates(path, coordinates);
    m_imagesModel->setChanged(path, true);
    m_mapWidget->addImage(path, coordinates);
}

void MainWindow::searchExactMatches(ImagesListView *list)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    const auto paths = list->selectedPaths();
    int matches = 0;
    QString lastMatchedPath;

    for (const auto &path : paths) {
        const auto coordinates = m_gpxEngine->findExactCoordinates(m_imagesModel->date(path),
                                                                   m_fixDriftWidget->deviation());
        if (coordinates.isSet) {
            m_imagesModel->setMatchType(path, KGeoTag::MatchType::Exact);
            assignImage(path, coordinates);
            matches++;
            lastMatchedPath = path;
        }
    }

    QApplication::restoreOverrideCursor();

    if (matches > 0) {
        m_mapWidget->reloadMap();
        m_mapWidget->centerImage(lastMatchedPath);
        m_previewWidget->setImage(lastMatchedPath);
        QMessageBox::information(this, i18n("Search for exact matches"),
            i18np("1 exact match found!", "%1 exact matches found!", matches));

    } else {
        QMessageBox::warning(this, i18n("Search for exact matches"),
            i18n("Could not find any exact matches!"));
    }
}

void MainWindow::searchInterpolatedMatches(ImagesListView *list)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    const auto paths = list->selectedPaths();
    int matches = 0;
    QString lastMatchedPath;

    QProgressDialog progress(i18n("Assigning images ..."), i18n("Cancel"), 0, paths.count(), this);
    progress.setWindowModality(Qt::WindowModal);

    int processed = 0;
    for (const auto &path : paths) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        const auto coordinates = m_gpxEngine->findInterpolatedCoordinates(
            m_imagesModel->date(path), m_fixDriftWidget->deviation());

        if (coordinates.isSet) {
            m_imagesModel->setMatchType(path, KGeoTag::MatchType::Interpolated);
            assignImage(path, coordinates);
            matches++;
            lastMatchedPath = path;
        }
    }

    progress.reset();
    QApplication::restoreOverrideCursor();

    if (matches > 0) {
        m_mapWidget->reloadMap();
        m_mapWidget->centerImage(lastMatchedPath);
        m_previewWidget->setImage(lastMatchedPath);
        QMessageBox::information(this, i18n("Search for interpolated matches"),
            i18np("1 interpolated match found!", "%1 interpolated matches found!", matches));

    } else {
        QMessageBox::warning(this, i18n("Search for interpolated matches"),
            i18n("Could not find any interpolated matches!"));
    }
}

void MainWindow::saveChanges()
{
    auto files = m_imagesModel->changedImages();

    if (files.isEmpty()) {
        QMessageBox::information(this, i18n("Save changes"), i18n("Nothing to do"));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // We sort the list so that the processing order matches the display order
    std::sort(files.begin(), files.end());

    const auto writeMode = s_writeModeMap.value(m_settings->writeMode());
    const bool createBackups =
        writeMode != KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOSIDECARONLY
        && m_settings->createBackups();
    const int deviation = m_fixDriftWidget->deviation();
    const bool fixDrift = m_fixDriftWidget->save() && deviation != 0;

    bool skipImage = false;
    bool abortWrite = false;
    int savedImages = 0;
    const int allImages = files.count();
    int processed = 0;
    const bool isSingleFile = allImages == 1;

    QProgressDialog progress(i18n("Saving changes ..."), i18n("Cancel"), 0, allImages, this);
    progress.setWindowModality(Qt::WindowModal);

    for (const QString &path : files) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        // Create a backup of the file if requested
        if (createBackups) {
            bool backupOkay = false;
            const QString backupPath = path + QStringLiteral(".orig");

            while (! backupOkay) {
                backupOkay = QFile::copy(path, backupPath);

                if (! backupOkay) {
                    progress.reset();
                    QApplication::restoreOverrideCursor();

                    QFileInfo info(path);

                    RetrySkipAbortDialog dialog(this,
                        i18n("Save changes"),
                        i18n("<p><b>Saving changes failed%1</b></p>"
                             "<p>Could not save changes to <kbd>%2</kbd>: The backup file <kbd>%3"
                             "</kbd> could not be created.</p>"
                             "<p>Please check if this file doesn't exist yet and be sure to have "
                             "write access to <kbd>%4</kbd>.</p>"
                             "<p>%5</p>",
                             isSingleFile ? QString()
                                          : i18nc("Fraction of processed files", " (%1 of %2)",
                                                  processed, allImages),
                             info.fileName(),
                             backupPath,
                             info.dir().path(),
                             isSingleFile ? i18n("You can retry to create the backup or cancel the "
                                                 "saving process.")
                                          : i18n("You can retry to create the backup, skip the "
                                                 "current file or cancel the saving process.")),
                             isSingleFile);

                    const auto reply = dialog.exec();
                    if (reply == RetrySkipAbortDialog::Skip) {
                        skipImage = true;
                        break;
                    } else if (reply == RetrySkipAbortDialog::Abort) {
                        abortWrite = true;
                        break;
                    }

                    QApplication::setOverrideCursor(Qt::WaitCursor);
                }
            }
        }

        if (skipImage) {
            skipImage = false;
            continue;
        }
        if (abortWrite) {
            break;
        }

        // Write the GPS information

        // Read the Exif header

        auto exif = KExiv2Iface::KExiv2();
        exif.setUseXMPSidecar4Reading(true);

        while (! exif.load(path)) {
            progress.reset();
            QApplication::restoreOverrideCursor();

            RetrySkipAbortDialog dialog(this,
                i18n("Save changes"),
                i18n("<p><b>Saving changes failed%1</b></p>"
                        "<p>Could not read exif header from <kbd>%2</kbd>.</p>"
                        "<p>Please check if this file still exists and if you have read access to "
                        "it (and possibly also to an existing XMP sidecar file).</p>"
                        "<p>%3</p>",
                        isSingleFile ? QString()
                                     : i18nc("Fraction of processed files", " (%1 of %2)",
                                             processed, allImages),
                        path,
                        isSingleFile ? i18n("You can retry to process the file or cancel the "
                                            "saving process.")
                                     : i18n("You can retry to process the file, skip it or cancel "
                                            "the saving process.")),
                        isSingleFile);

            const auto reply = dialog.exec();
            if (reply == RetrySkipAbortDialog::Skip) {
                skipImage = true;
                break;
            } else if (reply == RetrySkipAbortDialog::Abort) {
                abortWrite = true;
                break;
            }

            QApplication::setOverrideCursor(Qt::WaitCursor);
        }

        if (skipImage) {
            skipImage = false;
            continue;
        }
        if (abortWrite) {
            break;
        }

        // Set or remove the coordinates
        const auto coordinates = m_imagesModel->coordinates(path);
        if (coordinates.isSet) {
            exif.setGPSInfo(coordinates.alt, coordinates.lat, coordinates.lon);
        } else {
            exif.removeGPSInfo();
        }

        // Fix the time drift if requested
        if (fixDrift) {
            const QDateTime originalTime = m_imagesModel->date(path);
            const QDateTime fixedTime = originalTime.addSecs(deviation);
            // If the Digitization time is equal to the original time, update it as well.
            // Otherwise, only update the image's timestamp.
            exif.setImageDateTime(fixedTime, exif.getDigitizationDateTime() == originalTime);
        }

        // Save the changes

        exif.setMetadataWritingMode(writeMode);

        while (! exif.applyChanges()) {
            progress.reset();
            QApplication::restoreOverrideCursor();

            QString neededWritePermissions;
            if (writeMode == KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOSIDECARONLY) {
                QFileInfo info(path);
                neededWritePermissions = i18n("Please check if you have write access to <kbd>%1"
                                              "</kbd>", info.dir().path());
            } else {
                neededWritePermissions =  i18n("Please check if this file still exists and if you "
                                               "have write access to it.");
            }

            RetrySkipAbortDialog dialog(this,
                i18n("Save changes"),
                i18n("<p><b>Saving changes failed%1</b></p>"
                     "<p>Could not save metadata for <kbd>%2</kbd>.</p>"
                     "<p>%3""</p>"
                     "<p>%4</p>",
                     isSingleFile ? QString()
                                  : i18nc("Fraction of processed files", " (%1 of %2)",
                                          processed, allImages),
                     path,
                     neededWritePermissions,
                     isSingleFile ? i18n("You can retry to process the file or cancel the saving "
                                         "process.")
                                  : i18n("You can retry to process the file, skip it or cancel the "
                                         "saving process.")),
                     isSingleFile);

            const auto reply = dialog.exec();
            if (reply == RetrySkipAbortDialog::Skip) {
                skipImage = true;
                break;
            } else if (reply == RetrySkipAbortDialog::Abort) {
                abortWrite = true;
                break;
            }

            QApplication::setOverrideCursor(Qt::WaitCursor);
        }

        if (skipImage) {
            skipImage = false;
            continue;
        }
        if (abortWrite) {
            break;
        }

        m_imagesModel->setChanged(path, false);

        savedImages++;
    }

    progress.reset();
    QApplication::restoreOverrideCursor();

    if (savedImages == 0) {
        QMessageBox::warning(this, i18n("Save changes"),
                             i18n("No changes could be saved!"));
    } else if (savedImages < allImages) {
        QMessageBox::warning(this, i18n("Save changes"),
                             i18n("Some changes could not be saved. Successfully saved %1 of %2 "
                                  "images.", savedImages, allImages));
    } else {
        QMessageBox::information(this, i18n("Save changes"),
                                 i18n("All changes have been successfully saved!"));
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

void MainWindow::removeCoordinates(ImagesListView *list)
{
    const auto paths = list->selectedPaths();
    for (const QString &path : paths) {
        m_imagesModel->setCoordinates(path, KGeoTag::NoCoordinates);
        m_imagesModel->setChanged(path, true);
        m_imagesModel->setMatchType(path, KGeoTag::MatchType::None);
        m_mapWidget->removeImage(path);
    }

    m_mapWidget->reloadMap();
    m_previewWidget->setImage(m_previewWidget->currentImage());
}

void MainWindow::discardChanges(ImagesListView *list)
{
    const auto paths = list->selectedPaths();
    for (const auto &path : paths) {
        m_imagesModel->resetChanges(path);

        const auto coordinates = m_imagesModel->coordinates(path);
        if (coordinates.isSet) {
            m_mapWidget->addImage(path, coordinates);
        } else {
            m_mapWidget->removeImage(path);
        }

    }

    m_mapWidget->reloadMap();
    m_previewWidget->setImage(m_previewWidget->currentImage());
}

void MainWindow::checkUpdatePreview(const QVector<QString> &paths)
{
    for (const QString &path : paths) {
        if (m_previewWidget->currentImage() == path) {
            m_previewWidget->setImage(path);
            break;
        }
    }
}

void MainWindow::elevationLookupFailed(const QString &errorMessage)
{
    QApplication::restoreOverrideCursor();

    QMessageBox::warning(this, i18n("Elevation lookup"),
        i18n("<p>Fetching elevation data from opentopodata.org failed.</p>"
             "<p>The error message was: %1</p>", errorMessage));
}

void MainWindow::notAllElevationsPresent(int locationsCount, int elevationsCount)
{
    QString message;
    if (locationsCount == 1) {
        message = i18n("Fetching elevation data failed: The requested location is not present in "
                       "the currently chosen elevation dataset.");
    } else {
        if (elevationsCount == 0) {
            message = i18n("Fetching elevation data failed: None of the requested locations are "
                           "present in the currently chosen elevation dataset.");
        } else {
            message = i18n("Fetching elevation data is incomplete: Some of the requested locations "
                           "are not present in the currently chosen elevation dataset.");
        }
    }

    QMessageBox::warning(this, i18n("Elevation lookup"), message);
}

void MainWindow::lookupElevation(ImagesListView *list)
{
    lookupElevation(list->selectedPaths());
}

void MainWindow::lookupElevation(const QVector<QString> &paths)
{
    QApplication::setOverrideCursor(Qt::BusyCursor);

    QVector<KGeoTag::Coordinates> coordinates;
    for (const auto &path : paths) {
        coordinates.append(m_imagesModel->coordinates(path));
    }

    m_elevationEngine->request(ElevationEngine::Target::Image, paths, coordinates);
}

void MainWindow::elevationProcessed(ElevationEngine::Target target, const QVector<QString> &paths,
                                    const QVector<double> &elevations)
{
    if (target != ElevationEngine::Target::Image) {
        return;
    }

    for (int i = 0; i < paths.count(); i++) {
        const auto &path = paths.at(i);
        const auto &elevation = elevations.at(i);
        auto coordinates = m_imagesModel->coordinates(path);
        coordinates.alt = elevation;
        m_imagesModel->setCoordinates(path, coordinates);
    }

    emit checkUpdatePreview(paths);
    QApplication::restoreOverrideCursor();
}
