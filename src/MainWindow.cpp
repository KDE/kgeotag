/* SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tl@l3u.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
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
#include "Coordinates.h"
#include "AutomaticMatchingWidget.h"
#include "MimeHelper.h"
#include "MapCenterInfo.h"
#include "TracksListView.h"
#include "GeoDataModel.h"
#include "TrackWalker.h"

// KDE includes
#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardAction>
#include <KHelpMenu>
#include <KExiv2/KExiv2>
#include <KXMLGUIFactory>

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
#include <QVBoxLayout>

static const QHash<QString, KExiv2Iface::KExiv2::MetadataWritingMode> s_writeModeMap {
    { QStringLiteral("WRITETOIMAGEONLY"),
      KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOIMAGEONLY },
    { QStringLiteral("WRITETOSIDECARONLY"),
      KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOSIDECARONLY },
    { QStringLiteral("WRITETOSIDECARANDIMAGE"),
      KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOSIDECARANDIMAGE }
};

MainWindow::MainWindow(SharedObjects *sharedObjects)
    : KXmlGuiWindow(),
      m_sharedObjects(sharedObjects),
      m_settings(sharedObjects->settings()),
      m_gpxEngine(sharedObjects->gpxEngine()),
      m_elevationEngine(sharedObjects->elevationEngine()),
      m_imagesModel(sharedObjects->imagesModel()),
      m_geoDataModel(sharedObjects->geoDataModel())
{
    setWindowTitle(i18n("KGeoTag"));

    connect(m_elevationEngine, &ElevationEngine::elevationProcessed,
            this, &MainWindow::elevationProcessed);

    connect(m_geoDataModel, &GeoDataModel::requestAddFiles, this, &MainWindow::addGpx);

    // Menu setup
    // ==========

    // File
    // ----

    auto *addFilesAction = actionCollection()->addAction(QStringLiteral("addFiles"));
    addFilesAction->setText(i18n("Add images and/or GPX tracks"));
    addFilesAction->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    actionCollection()->setDefaultShortcut(addFilesAction, QKeySequence(tr("Ctrl+F")));
    connect(addFilesAction, &QAction::triggered, this, &MainWindow::addFiles);

    auto *addDirectoryAction = actionCollection()->addAction(QStringLiteral("addDirectory"));
    addDirectoryAction->setText(i18n("Add all images and tracks from directory"));
    addDirectoryAction->setIcon(QIcon::fromTheme(QStringLiteral("archive-insert-directory")));
    actionCollection()->setDefaultShortcut(addDirectoryAction, QKeySequence(tr("Ctrl+D")));
    connect(addDirectoryAction, &QAction::triggered, this, &MainWindow::addDirectory);

    // "Remove" submenu

    auto *removeProcessedSavedImagesAction
        = actionCollection()->addAction(QStringLiteral("removeProcessedSavedImages"));
    removeProcessedSavedImagesAction->setText(i18n("All processed and saved images"));
    connect(removeProcessedSavedImagesAction, &QAction::triggered,
            this, &MainWindow::removeProcessedSavedImages);

    auto removeImagesLoadedTaggedAction
        = actionCollection()->addAction(QStringLiteral("removeImagesLoadedTagged"));
    removeImagesLoadedTaggedAction->setText(i18n("All images that already had coordinates"));
    connect(removeImagesLoadedTaggedAction, &QAction::triggered,
            this, &MainWindow::removeImagesLoadedTagged);

    auto *removeAllImagesAction = actionCollection()->addAction(QStringLiteral("removeAllImages"));
    removeAllImagesAction->setText(i18n("All images"));
    connect(removeAllImagesAction, &QAction::triggered, this, &MainWindow::removeAllImages);

    auto *removeAllTracksAction = actionCollection()->addAction(QStringLiteral("removeAllTracks"));
    removeAllTracksAction->setText(i18n("All tracks"));
    connect(removeAllTracksAction, &QAction::triggered, this, &MainWindow::removeAllTracks);

    auto *removeEverything = actionCollection()->addAction(QStringLiteral("removeEverything"));
    removeEverything->setText(i18n("All images and tracks (reset)"));
    connect(removeEverything, &QAction::triggered, this, &MainWindow::removeEverything);

    // "File" menu again

    auto *searchMatchesAction = actionCollection()->addAction(QStringLiteral("searchMatches"));
    searchMatchesAction->setText(i18n("Assign images to GPS data"));
    searchMatchesAction->setIcon(QIcon::fromTheme(QStringLiteral("crosshairs")));
    actionCollection()->setDefaultShortcut(searchMatchesAction, QKeySequence(tr("Ctrl+A")));
    connect(searchMatchesAction, &QAction::triggered,
            [this]
            {
                triggerCompleteAutomaticMatching(m_settings->defaultMatchingMode());
            });

    auto *saveChangesAction = actionCollection()->addAction(QStringLiteral("saveChanges"));
    saveChangesAction->setText(i18n("Save changed images"));
    saveChangesAction->setIcon(QIcon::fromTheme(QStringLiteral("document-save-all")));
    actionCollection()->setDefaultShortcut(saveChangesAction, QKeySequence(tr("Ctrl+S")));
    connect(saveChangesAction, &QAction::triggered, this, &MainWindow::saveAllChanges);

    KStandardAction::quit(this, &QWidget::close, actionCollection());

    // Settings
    // --------

    auto *setDefaultDockArrangementAction
        = actionCollection()->addAction(QStringLiteral("setDefaultDockArrangement"));
    setDefaultDockArrangementAction->setText(i18n("Set default dock arrangement"));
    setDefaultDockArrangementAction->setIcon(QIcon::fromTheme(QStringLiteral("refactor")));
    connect(setDefaultDockArrangementAction, &QAction::triggered,
            this, &MainWindow::setDefaultDockArrangement);

    KStandardAction::preferences(this, &MainWindow::showSettings, actionCollection());

    setupGUI(Keys | Save | Create);

    // Elicit a pointer from the "remove" menu from the XmlGui ;-)
    // setupGUI() has to be called before this works
    auto *removeMenu = qobject_cast<QMenu *>(guiFactory()->container(QStringLiteral("remove"),
                                                                     this));
    removeMenu->menuAction()->setIcon(QIcon::fromTheme(QStringLiteral("document-close")));

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

    // Automatic matching
    m_automaticMatchingWidget = new AutomaticMatchingWidget(m_settings);
    m_automaticMatchingDock = createDockWidget(i18n("Automatic matching"),
                                               m_automaticMatchingWidget,
                                               QStringLiteral("automaticMatchingDock"));
    connect(m_automaticMatchingWidget, &AutomaticMatchingWidget::requestReassignment,
            this, &MainWindow::triggerCompleteAutomaticMatching);

    // Fix drift
    m_fixDriftWidget = new FixDriftWidget;
    m_fixDriftDock = createDockWidget(i18n("Fix time drift"), m_fixDriftWidget,
                                          QStringLiteral("fixDriftDock"));
    connect(m_fixDriftWidget, &FixDriftWidget::imagesTimeZoneChanged,
            this, &MainWindow::imagesTimeZoneChanged);
    connect(m_fixDriftWidget, &FixDriftWidget::cameraDriftSettingsChanged,
            this, &MainWindow::cameraDriftSettingsChanged);

    // Map

    m_mapWidget = m_sharedObjects->mapWidget();
    m_mapCenterInfo = new MapCenterInfo(m_sharedObjects);
    connect(m_mapWidget, &MapWidget::mapMoved, m_mapCenterInfo, &MapCenterInfo::mapMoved);

    auto *mapWrapper = new QWidget;
    auto *mapWrapperLayout = new QVBoxLayout(mapWrapper);
    mapWrapperLayout->addWidget(m_mapWidget);
    mapWrapperLayout->addWidget(m_mapCenterInfo);

    m_mapDock = createDockWidget(i18n("Map"), mapWrapper, QStringLiteral("mapDock"));

    connect(m_mapWidget, &MapWidget::imagesDropped, this, &MainWindow::imagesDropped);
    connect(m_mapWidget, &MapWidget::requestLoadGpx, this, &MainWindow::addGpx);

    // Images lists

    m_unAssignedImagesDock = createImagesDock(KGeoTag::UnAssignedImages, i18n("Unassigned images"),
                                              QStringLiteral("unAssignedImagesDock"));
    m_assignedOrAllImagesDock = createImagesDock(KGeoTag::AssignedImages, QString(),
                                                 QStringLiteral("assignedOrAllImagesDock"));
    updateImagesListsMode();

    // Tracks

    m_tracksView = new TracksListView(m_geoDataModel);
    connect(m_tracksView, &TracksListView::trackSelected, m_mapWidget, &MapWidget::zoomToTrack);
    connect(m_tracksView, &TracksListView::removeTracks, this, &MainWindow::removeTracks);

    auto *trackWalker = new TrackWalker(m_geoDataModel);
    connect(m_tracksView, &TracksListView::updateTrackWalker,
            trackWalker, &TrackWalker::setToTrack);
    connect(trackWalker, &TrackWalker::trackPointSelected, this, &MainWindow::centerTrackPoint);

    auto *tracksWrapper = new QWidget;
    auto *tracksWrapperLayout = new QVBoxLayout(tracksWrapper);
    tracksWrapperLayout->setContentsMargins(0, 0, 0, 0);
    tracksWrapperLayout->addWidget(m_tracksView);
    tracksWrapperLayout->addWidget(trackWalker);

    m_tracksDock = createDockWidget(i18n("Tracks"), tracksWrapper, QStringLiteral("tracksDock"));

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
    connect(list, &ImagesListView::requestAutomaticMatching,
            this, &MainWindow::triggerAutomaticMatching);
    connect(list, &ImagesListView::assignToMapCenter, this, &MainWindow::assignToMapCenter);
    connect(list, &ImagesListView::assignManually, this, &MainWindow::assignManually);
    connect(list, &ImagesListView::editCoordinates, this, &MainWindow::editCoordinates);
    connect(list, QOverload<ImagesListView *>::of(&ImagesListView::removeCoordinates),
            this, QOverload<ImagesListView *>::of(&MainWindow::removeCoordinates));
    connect(list, QOverload<const QVector<QString> &>::of(&ImagesListView::removeCoordinates),
            this, QOverload<const QVector<QString> &>::of(&MainWindow::removeCoordinates));
    connect(list, &ImagesListView::discardChanges, this, &MainWindow::discardChanges);
    connect(list, &ImagesListView::lookupElevation,
            this, QOverload<ImagesListView *>::of(&MainWindow::lookupElevation));
    connect(list, &ImagesListView::assignTo, this, &MainWindow::assignTo);
    connect(list, &ImagesListView::requestAddingImages, this, &MainWindow::addImages);
    connect(list, &ImagesListView::removeImages, this, &MainWindow::removeImages);
    connect(list, &ImagesListView::requestSaving, this, &MainWindow::saveSelection);

    return createDockWidget(title, list, dockId);
}

void MainWindow::updateImagesListsMode()
{
    if (m_settings->splitImagesList()) {
        m_assignedOrAllImagesDock->setWindowTitle(i18n("Assigned images"));
        qobject_cast<ImagesListView *>(
            m_assignedOrAllImagesDock->widget())->setListType(KGeoTag::AssignedImages);
        m_unAssignedImagesDock->show();
        qobject_cast<ImagesListView *>(
            m_unAssignedImagesDock->widget())->setListType(KGeoTag::UnAssignedImages);
        m_imagesModel->setSplitImagesList(true);
    } else {
        m_assignedOrAllImagesDock->setWindowTitle(i18n("Images"));
        qobject_cast<ImagesListView *>(
            m_assignedOrAllImagesDock->widget())->setListType(KGeoTag::AllImages);
        m_unAssignedImagesDock->hide();
        m_imagesModel->setSplitImagesList(false);
    }
}

void MainWindow::setDefaultDockArrangement()
{
    const QVector<QDockWidget *> allDocks = {
        m_assignedOrAllImagesDock,
        m_unAssignedImagesDock,
        m_previewDock,
        m_fixDriftDock,
        m_automaticMatchingDock,
        m_bookmarksDock,
        m_mapDock
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

    const QVector<QDockWidget *> toTabify = {
        m_previewDock,
        m_fixDriftDock,
        m_automaticMatchingDock,
        m_bookmarksDock
    };

    for (int i = 0; i < toTabify.count() - 1; i++) {
        tabifyDockWidget(toTabify.at(i), toTabify.at(i + 1));
    }
    toTabify.first()->raise();

    tabifyDockWidget(m_assignedOrAllImagesDock, m_tracksDock);
    m_assignedOrAllImagesDock->raise();

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
    if (! m_imagesModel->imagesWithPendingChanges().isEmpty()) {
        if (QMessageBox::question(this, i18n("Close KGeoTag"),
            i18n("<p>There are pending changes to images that haven't been saved yet. All changes "
                 "will be discarded if KGeoTag is closed now.</p>"
                 "<p>Do you want to close the program anyway?</p>"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {

            event->ignore();
            return;
        }
    }

    m_settings->saveMainWindowState(saveState());

    m_mapWidget->saveSettings();

    m_settings->saveBookmarks(m_bookmarksWidget->bookmarks());

    QApplication::quit();
}

void MainWindow::addFiles()
{
    const auto selection = QFileDialog::getOpenFileNames(this,
                                i18n("Please select the images and/or GPX tracks to add"),
                                m_settings->lastOpenPath(),
                                i18n("All supported files ("
                                    "*.jpg *.jpeg "
                                    "*.png "
                                    "*.webp "
                                    "*.tif *.tiff "
                                    "*.ora "
                                    "*.kra "
                                    "*.gpx "
                                    ");; All files (*)"));

    if (selection.isEmpty()) {
        return;
    }

    // Check the MIME type of all selected files
    QHash<KGeoTag::FileType, QVector<QString>> classified;
    for (const auto &path : selection) {
        classified[MimeHelper::classifyFile(path)].append(path);
    }

    // Inform the user if some unsupported files have been selected

    if (classified.value(KGeoTag::UnsupportedFile).count() > 0) {
        QString text;

        if (classified.value(KGeoTag::ImageFile).count() == 0
            && classified.value(KGeoTag::GeoDataFile).count() == 0) {

            text = i18n("<p>The selection did not contain any supported files!</p>");

        } else {
            QString skippedList;
            for (const auto &path : classified.value(KGeoTag::UnsupportedFile)) {
                QFileInfo info(path);
                skippedList.append(i18nc(
                    "A filename with a MIME type in braces and a HTML line break",
                    "%1 (%2)<br/>",
                    info.fileName(),
                    MimeHelper::mimeType(path)));
            }

            text = i18np("<p>The following file will be skipped due to an unsupported MIME type:"
                         "</p>"
                         "<p>%2</p>",
                         "<p>The following files will be skipped due to unsupported MIME types:</p>"
                         "<p>%2</p>",
                         classified.value(KGeoTag::UnsupportedFile).count(),
                         skippedList);
        }

        QMessageBox::warning(this, i18n("Add images and/or GPX tracks"), text);
    }

    // Add the geodata files
    if (classified.value(KGeoTag::GeoDataFile).count() > 0) {
        addGpx(classified.value(KGeoTag::GeoDataFile));
    }

    // Add the images
    if (classified.value(KGeoTag::ImageFile).count() > 0) {
        addImages(classified.value(KGeoTag::ImageFile));
    }
}

void MainWindow::addDirectory()
{
    const auto directory = QFileDialog::getExistingDirectory(this,
                               i18n("Please select a directory"),
                               m_settings->lastOpenPath());

    if (directory.isEmpty()) {
        return;
    }

    QDir dir(directory);
    const auto files = dir.entryList({ QStringLiteral("*") }, QDir::Files);

    QVector<QString> geoDataFiles;
    QVector<QString> images;
    for (const auto &file : files) {
        const auto path = directory + QStringLiteral("/") + file;
        switch (MimeHelper::classifyFile(path)) {
        case KGeoTag::GeoDataFile:
            geoDataFiles.append(path);
            break;
        case KGeoTag::ImageFile:
            images.append(path);
            break;
        case KGeoTag::UnsupportedFile:
            break;
        }
    }

    if (geoDataFiles.isEmpty() && images.isEmpty()) {
        QMessageBox::warning(this, i18n("Add all images and tracks from directory"),
                             i18n("Could not find any supported files in <kbd>%1</kbd>",
                                  directory));
        return;
    }

    // Add the geodata files
    if (geoDataFiles.count() > 0) {
        addGpx(geoDataFiles);
    }

    // Add the images
    if (images.count() > 0) {
        addImages(images);
    }
}

void MainWindow::addGpx(const QVector<QString> &paths)
{
    const int filesCount = paths.count();
    int processed = 0;
    int failed = 0;
    int allFiles = 0;
    int allTracks = 0;
    int allSegments = 0;
    int allPoints = 0;
    int alreadyLoaded = 0;
    QVector<QString> loadedPaths;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    for (const auto &path : paths) {
        processed++;

        const QFileInfo info(path);
        m_settings->saveLastOpenPath(info.dir().absolutePath());

        const auto [ result, tracks, segments, points ]
            = m_gpxEngine->load(info.canonicalFilePath());

        QString errorString;

        switch (result) {
        case GpxEngine::Okay:
            allFiles++;
            allTracks += tracks;
            allSegments += segments;
            allPoints += points;
            loadedPaths.append(path);
            break;

        case GpxEngine::AlreadyLoaded:
            alreadyLoaded++;
            break;

        case GpxEngine::OpenFailed:
            errorString = i18n("<p>Opening <kbd>%1</kbd> failed. Please be sure to have read "
                               "access to this file.</p>", path);
            break;

        case GpxEngine::NoGpxElement:
            errorString = i18n("<p>Could not read geodata from <kbd>%1</kbd>: Could not find the "
                               "<kbd>gpx</kbd> root element. Apparently, this is not a GPX file!"
                               "</p>", path);
            break;

        case GpxEngine::NoGeoData:
            errorString = i18n("<p><kbd>%1</kbd> seems to be a GPX file, but it contains no "
                               "geodata!</p>", path);
            break;

        case GpxEngine::XmlError:
            errorString = i18n("<p>XML parsing failed for <kbd>%1</kbd>. Either, no data could be "
                               "loaded at all, or only a part of it.</p>", path);
            break;
        }

        if (! errorString.isEmpty()) {
            failed++;

            QString text;
            if (filesCount == 1) {
                text = i18n("<p><b>Loading GPX file failed</b></p>");
            } else {
                text = i18nc("Fraction of processed files are added inside the round braces",
                             "<p><b>Loading GPX file failed (%1/%2)</b></p>",
                             processed, filesCount);
            }

            text.append(errorString);

            if (processed < filesCount) {
                text.append(i18n("<p>The next GPX file will be loaded now.</p>"));
            }

            QApplication::restoreOverrideCursor();
            QMessageBox::warning(this, i18n("Load GPX data"), text);
            QApplication::setOverrideCursor(Qt::WaitCursor);
        }
    }

    m_mapWidget->zoomToTracks(loadedPaths);

    QString text;

    if (failed == 0 && alreadyLoaded == 0) {
        text = i18np("<p>Processed one file</p>", "<p>Processed %1 files</p>", processed);
    } else if (failed > 0 && alreadyLoaded == 0) {
        text = i18ncp("Processed x files message with some files that failed to load. The "
                      "pluralized string for the failed files counter (%2) is provided by the "
                      "following i18np call.",
                      "<p>Processed one file; %2</p>", "<p>Processed %1 files; %2</p>",
                      processed,
                      i18np("one file failed to load", "%1 files failed to load", failed));
    } else if (failed == 0 && alreadyLoaded > 0) {
        text = i18ncp("Processed x files message with some files that have been skipped. The "
                      "pluralized string for the skipped files counter (%2) is provided by the "
                      "following i18np call.",
                      "<p>Processed one file; %2</p>", "<p>Processed %1 files; %2</p>",
                      processed,
                      i18np("one already loaded file has been skipped",
                            "%1 already loaded files have been skipped", alreadyLoaded));
    } else if (failed > 0 && alreadyLoaded == 0) {
        text = i18ncp("Processed x files message with some files that failed to load and some that "
                      "have been skipped. The pluralized strings for the failed (%2) and skipped "
                      "files counter (%3) are provided by the following i18np call.",
                      "<p>Processed one file; %2, %3</p>", "<p>Processed %1 files; %2, %3</p>",
                      processed,
                      i18np("one file failed to load", "%1 files failed to load", failed),
                      i18np("one already loaded file has been skipped",
                            "%1 already loaded files have been skipped", alreadyLoaded));
    }

    if (allPoints == 0) {
        text.append(i18n("<p>No waypoints could be loaded.</p>"));
    } else {
        if (allTracks > 0 && allSegments == allTracks) {
            text.append(i18ncp(
                "Loaded x waypoints message with the number of tracks that have been read. The "
                "pluralized string for the tracks (%2) is provided by the following i18np call.",
                "<p>Loaded one waypoint from %2</p>",
                "<p>Loaded %1 waypoints from %2</p>",
                allPoints,
                i18np("one track", "%1 tracks", allTracks)));
        } else if (allTracks > 0 && allSegments != allTracks) {
            text.append(i18ncp(
                "Loaded x waypoints message with the number of tracks and the number of segments "
                "that have been read. The pluralized string for the tracks (%2) and the one for "
                "the segments (%3) are provided by the following i18np calls.",
                "<p>Loaded one waypoint from %2 and %3</p>",
                "<p>Loaded %1 waypoints from %2 and %3</p>",
                allPoints,
                i18np("one track", "%1 tracks", allTracks),
                i18np("one segment", "%1 segments", allSegments)));
        }
    }

    QApplication::restoreOverrideCursor();

    // Display the load result
    QMessageBox::information(this, i18n("Load GPX data"), text);

    // Adopt the detected timezone

    const QByteArray &id = m_gpxEngine->lastDetectedTimeZoneId();

    if (allTracks > 0 && id != m_fixDriftWidget->imagesTimeZoneId()) {
        if (id.isEmpty()) {
            QMessageBox::warning(this, i18n("Timezone detection failed"),
                i18n("<p>The presumably correct timezone for images associated with the loaded GPX "
                     "file could not be detected.</p>"
                     "<p>Please set the correct timezone manually on the \"Fix time drift\" page."
                     "</p>"));
        } else {
            if (! m_fixDriftWidget->setImagesTimeZone(id)) {
                QMessageBox::warning(this, i18n("Setting the detected timezone failed"),
                    i18n("<p>The presumably correct timezone \"%1\" has been detected from the "
                         "loaded GPX file, but could not be set. This should not happen!</p>"
                         "<p>Please file a bug report about this, including the used version of "
                         "KGeoTag and Qt!</p>"
                         "<p>You can adjust the timezone setting manually on the \"Fix time "
                         "drift\" page.</p>",
                         QString::fromLatin1(id)));
            } else {
                QMessageBox::information(this, i18n("Timezone adjusted"),
                    i18n("<p>The loaded GPX file was presumably recorded in the timezone \"%1\", "
                         "as well as the photos to associate with it. This timezone has been "
                         "selected now.</p>"
                         "<p>You can adjust the timezone setting manually on the \"Fix time "
                         "drift\" page.</p>",
                         QString::fromLatin1(id)));
            }
        }
    }
}

void MainWindow::addImages(const QVector<QString> &paths)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    const QFileInfo info(paths.at(0));
    m_settings->saveLastOpenPath(info.dir().absolutePath());

    const int requested = paths.count();
    const bool isSingleFile = requested == 1;
    int processed = 0;
    int loaded = 0;
    int alreadyLoaded = 0;
    bool skipImage = false;
    bool abortLoad = false;

    QProgressDialog progress(i18n("Loading images ..."), i18n("Cancel"), 0, requested, this);
    progress.setWindowModality(Qt::WindowModal);

    for (const auto &path : paths) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        const QFileInfo info(path);
        while (true) {
            QString errorString;
            bool exitLoop = false;

            switch (m_imagesModel->addImage(info.canonicalFilePath())) {
            case ImagesModel::LoadingSucceeded:
                exitLoop = true;
                break;

            case ImagesModel::AlreadyLoaded:
                alreadyLoaded++;
                exitLoop = true;
                break;

            case ImagesModel::LoadingImageFailed:
                if (isSingleFile) {
                    errorString = i18n("<p><b>Loading image failed</b></p>"
                                       "<p>Could not read <kbd>%1</kbd>.</p>",
                                       path);
                } else {
                    errorString = i18nc(
                        "Message with a fraction of processed files added in round braces",
                        "<p><b>Loading image failed (%1/%2)</b></p>"
                        "<p>Could not read <kbd>%3</kbd>.</p>",
                        processed, requested, path);
                }
                break;

            case ImagesModel::LoadingMetadataFailed:
                if (isSingleFile) {
                    errorString = i18n(
                        "<p><b>Loading image's Exif header or XMP sidecar file failed</b></p>"
                        "<p>Could not read <kbd>%1</kbd>.</p>",
                        path);
                } else {
                    errorString = i18nc(
                        "Message with a fraction of processed files added in round braces",
                        "<p><b>Loading image's Exif header or XMP sidecar file failed</b></p>"
                        "<p>Could not read <kbd>%2</kbd>.</p>",
                        processed, requested, path);
                }
                break;

            }

            if (exitLoop || errorString.isEmpty()) {
                break;
            }

            errorString.append(i18n("<p>Please check if this file is actually a supported image "
                                    "and if you have read access to it.</p>"));

            if (isSingleFile) {
                errorString.append(i18n("<p>You can retry to load this file or cancel the loading "
                                        "process.</p>"));
            } else {
                errorString.append(i18n("<p>You can retry to load this file, skip it or cancel the "
                                        "loading process.</p>"));
            }

            progress.reset();
            QApplication::restoreOverrideCursor();

            RetrySkipAbortDialog dialog(this, i18n("Add images"), errorString, isSingleFile);
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

        loaded++;
    }

    progress.reset();
    m_mapWidget->reloadMap();
    QApplication::restoreOverrideCursor();

    const int failed = requested - loaded;
    loaded -= alreadyLoaded;

    if (loaded == requested) {
        QMessageBox::information(this, i18n("Add images"),
                                 i18np("Successfully added one image!",
                                       "Successfully added %1 images!",
                                       loaded));

    } else if (failed == requested) {
        QMessageBox::warning(this, i18n("Add images"), i18n(
            "Could not add any new images, all requested images failed to load!"));

    } else if (alreadyLoaded == requested) {
        QMessageBox::warning(this, i18n("Add images"), i18n(
            "Could not add any new images, all requested images have already been loaded!"));

    } else if (alreadyLoaded + failed == requested) {
        QMessageBox::warning(this, i18n("Add images"), i18n(
            "Could not add any new images, all requested images failed to load or have already "
            "been loaded!"));

    } else {
        QString message = i18np("<p>Successfully added image!</p>",
                                "<p>Successfully added %1 images!</p>",
                                loaded);

        if (failed > 0 && alreadyLoaded == 0) {
            message.append(i18np("<p>One image failed to load.</p>",
                                 "<p>%1 images failed to load.</p>",
                                 failed));
        } else if (failed == 0 && alreadyLoaded > 0) {
            message.append(i18np("<p>One image has already been loaded.</p>",
                                 "<p>%1 images have already been loaded.</p>",
                                 alreadyLoaded));
        } else {
            message.append(i18nc(
                "Message string for some images that failed to load and some that were skipped "
                "because they already have been loaded. The pluralized strings for the failed "
                "images (%1) and the skipped images (%2) are provided by the following i18np "
                "calls.",
                "<p>%1 and %2.</p>",
                i18np("One image failed to load",
                      "%1 images failed to load",
                      failed),
                i18np("one image has already been loaded",
                      "%1 images have already been loaded",
                      alreadyLoaded)));
        }

        QMessageBox::warning(this, i18n("Add images"), message);
    }
}

void MainWindow::imagesDropped(const QVector<QString> &paths)
{
    m_previewWidget->setImage(m_imagesModel->indexFor(paths.last()));
    if (m_settings->lookupElevationAutomatically()) {
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
                             m_settings->lookupElevationAutomatically(), Coordinates(), label);
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
                             identicalCoordinates ? coordinates : Coordinates(),
                             label);
    if (! dialog.exec()) {
        return;
    }

    assignTo(paths, dialog.coordinates());
}

void MainWindow::assignTo(const QVector<QString> &paths, const Coordinates &coordinates)
{
    for (const auto &path : paths) {
        m_imagesModel->setCoordinates(path, coordinates, KGeoTag::ManuallySet);
    }

    m_mapWidget->centerCoordinates(coordinates);
    m_mapWidget->reloadMap();

    if (m_settings->lookupElevationAutomatically()) {
        lookupElevation(paths);
    }
}

void MainWindow::triggerAutomaticMatching(ImagesListView *list, KGeoTag::SearchType searchType)
{
    const auto paths = list->selectedPaths();
    matchAutomatically(paths, searchType);
}

void MainWindow::triggerCompleteAutomaticMatching(KGeoTag::SearchType searchType)
{
    if (m_imagesModel->allImages().isEmpty()) {
        QMessageBox::information(this, i18n("(Re)Assign all images"),
                                 i18n("Can't search for matches:\n"
                                      "No images have been loaded yet."));
        return;
    }

    QVector<QString> paths;
    const bool excludeManuallyTagged = m_automaticMatchingWidget->excludeManuallyTagged();
    for (const auto &path : m_imagesModel->allImages()) {
        if (excludeManuallyTagged && m_imagesModel->matchType(path) == KGeoTag::ManuallySet) {
            continue;
        }
        paths.append(path);
    }
    matchAutomatically(paths, searchType);
}

void MainWindow::matchAutomatically(const QVector<QString> &paths, KGeoTag::SearchType searchType)
{
    if (m_geoDataModel->rowCount() == 0) {
        QMessageBox::information(this, i18n("Automatic matching"),
                                 i18n("Can't search for matches:\n"
                                      "No GPS tracks have been loaded yet."));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_gpxEngine->setMatchParameters(m_automaticMatchingWidget->exactMatchTolerance(),
                                    m_automaticMatchingWidget->maximumInterpolationInterval(),
                                    m_automaticMatchingWidget->maximumInterpolationDistance());

    int exactMatches = 0;
    int interpolatedMatches = 0;
    QString lastMatchedPath;

    QProgressDialog progress(i18n("Assigning images ..."), i18n("Cancel"), 0, paths.count(), this);
    progress.setWindowModality(Qt::WindowModal);

    int processed = 0;
    int notMatched = 0;
    int notMatchedButHaveCoordinates = 0;

    for (const auto &path : paths) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        Coordinates coordinates;

        // Search for exact matches if requested

        if (searchType == KGeoTag::CombinedMatchSearch
            || searchType == KGeoTag::ExactMatchSearch) {

            coordinates = m_gpxEngine->findExactCoordinates(
                m_imagesModel->date(path), m_fixDriftWidget->cameraClockDeviation());
        }

        if (coordinates.isSet()) {
            m_imagesModel->setCoordinates(path, coordinates, KGeoTag::ExactMatch);
            exactMatches++;
            lastMatchedPath = path;
            continue;
        }

        // Search for interpolated matches if requested

        if (searchType == KGeoTag::CombinedMatchSearch
            || searchType == KGeoTag::InterpolatedMatchSearch) {

            coordinates = m_gpxEngine->findInterpolatedCoordinates(
                m_imagesModel->date(path), m_fixDriftWidget->cameraClockDeviation());
        }

        if (coordinates.isSet()) {
            m_imagesModel->setCoordinates(path, coordinates, KGeoTag::InterpolatedMatch);
            interpolatedMatches++;
            lastMatchedPath = path;
        } else {
            notMatched++;
            if (m_imagesModel->coordinates(path).isSet()) {
                notMatchedButHaveCoordinates++;
            }
        }
    }

    progress.reset();

    QString title;
    QString text;

    switch (searchType) {
    case KGeoTag::CombinedMatchSearch:
        title = i18n("Combined match search");
        if (exactMatches > 0 || interpolatedMatches > 0) {
            text = i18nc("Message for the number of matches found. The pluralized string for the "
                         "exact matches (%1) and the interpolated matches (%2) are provided by the "
                         "following i18np calls.",
                         "<p>Found %1 and %2!</p>",
                         i18np("one exact match",
                               "%1 exact matches",
                               exactMatches),
                         i18np("one interpolated match",
                               "%1 interpolated matches",
                               interpolatedMatches));
            if (notMatched > 0) {
                text.append(i18ncp("Message for the number of unmatched images. The number of "
                                   "images that could not be matched but already have coordinates "
                                   "assigned (%2) is provided by the following i18np call",
                                   "<p>One image could not be matched (%2).</p>",
                                   "<p>%1 images could not be matched (%2).</p>",
                                   notMatched,
                                   i18np("of which one image already has coordinates assigned",
                                         "of which %1 images already have coordinates assigned",
                                         notMatchedButHaveCoordinates)));
            }
        } else {
            text = i18n("Could neither find any exact, nor any interpolated matches!");
        }
        break;

    case KGeoTag::ExactMatchSearch:
        title = i18n("Exact matches search");
        if (exactMatches > 0) {
            text = i18np("<p>Found one exact match!</p>",
                         "<p>Found %1 exact matches!</p>",
                         exactMatches);
            if (notMatched > 0) {
                text.append(i18ncp("Message for the number of unmatched images. The number of "
                                   "images that could not be matched but already have coordinates "
                                   "assigned (%2) is provided by the following i18np call",
                                   "<p>One image had no exact match (%2).</p>",
                                   "<p>%1 images had no exact match (%2).</p>",
                                   notMatched,
                                   i18np("of which one image already has coordinates assigned",
                                         "of which %1 images already have coordinates assigned",
                                         notMatchedButHaveCoordinates)));
            }
        } else {
            text = i18n("Could not find any exact matches!");
        }
        break;

    case KGeoTag::InterpolatedMatchSearch:
        title = i18n("Interpolated matches search");
        if (interpolatedMatches > 0) {
            text = i18np("<p>Found one interpolated match!</p>",
                         "<p>Found %1 interpolated matches!</p>",
                         interpolatedMatches);
            if (notMatched > 0) {
                text.append(i18ncp("Message for the number of unmatched images. The number of "
                                   "images that could not be matched but already have coordinates "
                                   "assigned (%2) is provided by the following i18np call",
                                   "<p>One image had no interpolated match (%2).</p>",
                                   "<p>%1 images had no interpolated match (%2).</p>",
                                   notMatched,
                                   i18np("of which one image already has coordinates assigned",
                                         "of which %1 images already have coordinates assigned",
                                         notMatchedButHaveCoordinates)));
            }
        } else {
            text = i18n("Could not find any interpolated matches!");
        }
        break;
    }

    QApplication::restoreOverrideCursor();

    if (exactMatches > 0 || interpolatedMatches > 0) {
        m_mapWidget->reloadMap();
        const auto index = m_imagesModel->indexFor(lastMatchedPath);
        m_mapWidget->centerImage(index);
        m_previewWidget->setImage(index);
        QMessageBox::information(this, title, text);
    } else {
        QMessageBox::warning(this, title, text);
    }
}

QString MainWindow::saveFailedHeader(int processed, int allImages) const
{
    if (allImages == 1) {
        return i18n("<p><b>Saving changes failed</b></p>");
    } else {
        return i18nc("Saving failed message with the fraction of processed files given in the "
                     "round braces",
                     "<p><b>Saving changes failed (%1/%2)</b></p>",
                     processed, allImages);
    }
}

QString MainWindow::skipRetryCancelText(int processed, int allImages) const
{
    if (allImages == 1 || processed == allImages) {
        return i18n("<p>You can retry to process the file or cancel the saving process.</p>");
    } else {
        return i18n("<p>You can retry to process the file, skip it or cancel the saving process."
                    "</p>");
    }
}

void MainWindow::saveSelection(ImagesListView *list)
{
    QVector<QString> files;
    auto selected = list->selectedPaths();
    for (const auto &path : selected) {
        if (m_imagesModel->hasPendingChanges(path)) {
            files.append(path);
        }
    }
    saveChanges(files);
}

void MainWindow::saveAllChanges()
{
    saveChanges(m_imagesModel->imagesWithPendingChanges());
}

void MainWindow::saveChanges(const QVector<QString> &files)
{
    if (files.isEmpty()) {
        QMessageBox::information(this, i18n("Save changes"), i18n("Nothing to do"));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    const auto writeMode = s_writeModeMap.value(m_settings->writeMode());
    const bool createBackups =
        writeMode != KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOSIDECARONLY
        && m_settings->createBackups();
    const int cameraClockDeviation = m_fixDriftWidget->cameraClockDeviation();
    const bool fixDrift = m_fixDriftWidget->save() && cameraClockDeviation != 0;

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
            const QString backupPath = path + QStringLiteral(".") + KGeoTag::backupSuffix;

            while (! backupOkay) {
                backupOkay = QFile::copy(path, backupPath);

                if (! backupOkay) {
                    QFileInfo info(path);
                    QString message = saveFailedHeader(processed, allImages);

                    message.append(i18n(
                        "<p>Could not save changes to <kbd>%1</kbd>: The backup file <kbd>%2</kbd> "
                        "could not be created.</p>"
                        "<p>Please check if this file doesn't exist yet and be sure to have write "
                        "access to <kbd>%3</kbd>.</p>",
                        info.fileName(), backupPath, info.dir().path()));

                    message.append(skipRetryCancelText(processed, allImages));

                    progress.reset();
                    QApplication::restoreOverrideCursor();

                    RetrySkipAbortDialog dialog(this, i18n("Save changes"), message,
                                                isSingleFile || processed == allImages);

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
            QString message = saveFailedHeader(processed, allImages);

            message.append(i18n(
                "<p>Could not read metadata from <kbd>%1</kbd>.</p>"
                "<p>Please check if this file still exists and if you have read access to it (and "
                "possibly also to an existing XMP sidecar file).</p>",
                path));

            message.append(skipRetryCancelText(processed, allImages));

            progress.reset();
            QApplication::restoreOverrideCursor();

            RetrySkipAbortDialog dialog(this, i18n("Save changes"), message,
                                        isSingleFile || processed == allImages);

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
        if (coordinates.isSet()) {
            exif.setGPSInfo(coordinates.alt(), coordinates.lat(), coordinates.lon());
        } else {
            exif.removeGPSInfo();
        }

        // Fix the time drift if requested
        if (fixDrift) {
            const QDateTime originalTime = m_imagesModel->date(path);
            const QDateTime fixedTime = originalTime.addSecs(cameraClockDeviation);
            // If the Digitization time is equal to the original time, update it as well.
            // Otherwise, only update the image's timestamp.
            exif.setImageDateTime(fixedTime, exif.getDigitizationDateTime() == originalTime);
        }

        // Save the changes

        exif.setMetadataWritingMode(writeMode);

        while (! exif.applyChanges()) {
            QString message = saveFailedHeader(processed, allImages);

            message.append(i18n("<p>Could not save metadata for <kbd>%1</kbd>.</p>", path));

            if (writeMode == KExiv2Iface::KExiv2::MetadataWritingMode::WRITETOSIDECARONLY) {
                QFileInfo info(path);
                message.append(i18n("<p>Please check if you have write access to <kbd>%1</kbd>!",
                                    info.dir().path()));
            } else {
                message.append(i18n("<p>Please check if this file still exists and if you have "
                                    "write access to it!</p>"));
            }

            message.append(skipRetryCancelText(processed, allImages));

            progress.reset();
            QApplication::restoreOverrideCursor();

            RetrySkipAbortDialog dialog(this, i18n("Save changes"), message, isSingleFile);

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

        m_imagesModel->setSaved(path);

        savedImages++;
    }

    progress.reset();
    QApplication::restoreOverrideCursor();

    if (savedImages == 0) {
        QMessageBox::warning(this, i18n("Save changes"),
                             i18n("No changes could be saved!"));
    } else if (savedImages < allImages) {
        QMessageBox::warning(this, i18n("Save changes"),
                             i18n("<p>Some changes could not be saved!</p>"
                                  "<p>Successfully saved %1 of %2 images.</p>",
                                  savedImages, allImages));
    } else {
        QMessageBox::information(this, i18n("Save changes"),
                                 i18n("All changes have been successfully saved!"));
    }
}

void MainWindow::showSettings()
{
    auto *dialog = new SettingsDialog(m_settings, this);
    connect(dialog, &SettingsDialog::imagesListsModeChanged,
            this, &MainWindow::updateImagesListsMode);

    if (! dialog->exec()) {
        return;
    }

    m_mapWidget->updateSettings();
}

void MainWindow::removeCoordinates(ImagesListView *list)
{
    removeCoordinates(list->selectedPaths());
}

void MainWindow::removeCoordinates(const QVector<QString> &paths)
{
    for (const QString &path : paths) {
        m_imagesModel->setCoordinates(path, Coordinates(), KGeoTag::NotMatched);
    }

    m_mapWidget->reloadMap();
    m_previewWidget->setImage();
}

void MainWindow::discardChanges(ImagesListView *list)
{
    const auto paths = list->selectedPaths();
    for (const auto &path : paths) {
        m_imagesModel->resetChanges(path);
    }

    m_mapWidget->reloadMap();
    m_previewWidget->setImage();
}

void MainWindow::checkUpdatePreview(const QVector<QString> &paths)
{
    for (const QString &path : paths) {
        if (m_previewWidget->currentImage() == path) {
            m_previewWidget->setImage(m_imagesModel->indexFor(path));
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

    QVector<Coordinates> coordinates;
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
        m_imagesModel->setElevation(path, elevation);
    }

    emit checkUpdatePreview(paths);
    QApplication::restoreOverrideCursor();
}

void MainWindow::imagesTimeZoneChanged()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_imagesModel->setImagesTimeZone(m_fixDriftWidget->imagesTimeZoneId());
    m_previewWidget->reload();
    QApplication::restoreOverrideCursor();
}

void MainWindow::cameraDriftSettingsChanged()
{
    m_previewWidget->setCameraClockDeviation(
        m_fixDriftWidget->displayFixed() ? m_fixDriftWidget->cameraClockDeviation() : 0);
}

void MainWindow::removeImages(ImagesListView *list)
{
    const auto paths = list->selectedPaths();
    const auto count = paths.count();

    bool pendingChanges = false;
    for (const auto &path : paths) {
        if (m_imagesModel->hasPendingChanges(path)) {
            pendingChanges = true;
            break;
        }
    }

    if (pendingChanges) {
        if (QMessageBox::question(this, i18np("Remove image", "Remove images", count),
                i18np("The image has pending changes! Do you really want to remove it and discard "
                      "the changes?",
                      "At least one of the images has pending changes. Do you really want to "
                      "remove them and discard all changes?",
                      count),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {

            return;
        }
    }

    m_imagesModel->removeImages(paths);
    m_mapWidget->reloadMap();
    m_previewWidget->setImage();
}

void MainWindow::removeProcessedSavedImages()
{
    const auto paths = m_imagesModel->processedSavedImages();
    if (paths.isEmpty()) {
        QMessageBox::information(this, i18n("Remove all processed and saved images"),
            i18n("Nothing to do"));
        return;
    }

    m_imagesModel->removeImages(paths);
    m_mapWidget->reloadMap();
    m_previewWidget->setImage();
    QMessageBox::information(this, i18n("Remove all processed and saved images"),
        i18np("Removed one image", "Removed %1 images", paths.count()));
}

void MainWindow::removeImagesLoadedTagged()
{
    const auto paths = m_imagesModel->imagesLoadedTagged();
    if (paths.isEmpty()) {
        QMessageBox::information(this, i18n("Remove images that already had coordinates"),
            i18n("Nothing to do"));
        return;
    }

    m_imagesModel->removeImages(paths);
    m_mapWidget->reloadMap();
    m_previewWidget->setImage();
    QMessageBox::information(this, i18n("Remove images that already had coordinates"),
        i18np("Removed one image", "Removed %1 images", paths.count()));
}

bool MainWindow::checkForPendingChanges()
{
    if (! m_imagesModel->imagesWithPendingChanges().isEmpty()
        && QMessageBox::question(this, i18n("Remove all images"),
               i18n("<p>There are pending changes to images that haven't been saved yet. All "
                    "changes will be discarded if all images are removed now.</p>"
                    "<p>Do you really want to remove all images anyway?</p>"),
               QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {

        return false;
    }

    return true;
}

void MainWindow::removeAllImages()
{
    if (m_imagesModel->rowCount() == 0) {
        QMessageBox::information(this, i18n("Remove all images"), i18n("Nothing to do"));
        return;
    }

    if (! checkForPendingChanges()) {
        return;
    }

    m_imagesModel->removeAllImages();
    m_mapWidget->reloadMap();
    m_previewWidget->setImage();
}

void MainWindow::removeTracks()
{
    m_tracksView->blockSignals(true);
    const auto allRows = m_tracksView->selectedTracks();
    for (int row : allRows) {
        m_geoDataModel->removeTrack(row);
    }
    m_tracksView->blockSignals(false);
    m_mapWidget->reloadMap();
}

void MainWindow::removeAllTracks()
{
    m_tracksView->blockSignals(true);
    m_geoDataModel->removeAllTracks();
    m_tracksView->blockSignals(false);
    m_mapWidget->reloadMap();
}

void MainWindow::removeEverything()
{
    if (m_imagesModel->rowCount() == 0 && m_geoDataModel->rowCount() == 0) {
        QMessageBox::information(this, i18n("Remove all images and tracks (reset)"),
                                 i18n("Nothing to do"));
        return;
    }

    if (! checkForPendingChanges()) {
        return;
    }

    m_imagesModel->removeAllImages();
    m_previewWidget->setImage();
    removeAllTracks();
}

void MainWindow::centerTrackPoint(int trackIndex, int trackPointIndex)
{
    const auto dateTime = m_geoDataModel->dateTimes().at(trackIndex).at(trackPointIndex)
                              .toTimeZone(QTimeZone(m_fixDriftWidget->imagesTimeZoneId()));
    const auto coordinates = m_geoDataModel->trackPoints().at(trackIndex).value(dateTime);
    m_mapWidget->blockSignals(true);
    m_mapWidget->centerCoordinates(coordinates);
    m_mapCenterInfo->trackPointCentered(coordinates, dateTime);
    m_mapWidget->blockSignals(false);
}
