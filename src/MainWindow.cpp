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
#include "MainWindow.h"
#include "Settings.h"
#include "ImageCache.h"
#include "PreviewWidget.h"
#include "MapWidget.h"
#include "KGeoTag.h"
#include "ImagesList.h"
#include "SettingsDialog.h"

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

// C++ includes
#include <algorithm>

MainWindow::MainWindow() : QMainWindow()
{
    setWindowTitle(i18n("KGeoTag"));

    m_settings = new Settings(this);
    m_imageCache = new ImageCache(this, m_settings);

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

    auto *showSettingsAction = editMenu->addAction(i18n("Settings"));
    connect(showSettingsAction, &QAction::triggered, this, &MainWindow::showSettings);

    // Help
    auto *helpMenu = new KHelpMenu;
    menuBar()->addMenu(helpMenu->menu());

    // Dock setup
    setDockNestingEnabled(true);

    // Unassigned images
    m_unAssignedImages = new ImagesList(m_imageCache);
    auto *unassignedImagesDock = createDockWidget(i18n("Unassigned images"), m_unAssignedImages,
                                                  QStringLiteral("unassignedImagesDock"));
    connect(m_unAssignedImages, &ImagesList::removeCoordinates,
            this, &MainWindow::removeCoordinates);
    connect(m_unAssignedImages, &ImagesList::discardChanges, this, &MainWindow::discardChanges);

    // Assigned images
    m_assignedImages = new ImagesList(m_imageCache);
    auto *assignedImagesDock = createDockWidget(i18n("Assigned images"), m_assignedImages,
                                                QStringLiteral("assignedImagesDock"));
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

    // Map
    m_mapWidget = new MapWidget(m_settings, m_imageCache);
    createDockWidget(i18n("Map"), m_mapWidget, QStringLiteral("mapDock"));
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
        splitDockWidget(unassignedImagesDock, assignedImagesDock, Qt::Vertical);
        splitDockWidget(assignedImagesDock, previewDock, Qt::Vertical);
        splitDockWidget(assignedImagesDock, unassignedImagesDock, Qt::Horizontal);
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

void MainWindow::closeEvent(QCloseEvent *)
{
    m_settings->saveMainWindowGeometry(saveGeometry());
    m_settings->saveMainWindowState(saveState());

    m_mapWidget->saveSettings();

    QApplication::quit();
}

void MainWindow::addGpx()
{
    const auto file = QFileDialog::getOpenFileName(this,
                          i18n("Please select the GPX track to add"),
                          m_settings->lastOpenPath(),
                          i18n("GPX tracks (*.gpx)"));
    if (file.isEmpty()) {
        return;
    }

    const QFileInfo info(file);
    m_settings->saveLastOpenPath(info.dir().absolutePath());
    m_mapWidget->addGpx(file);
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
    const QFileInfo info(path);
    m_unAssignedImages->removeImage(path);
    m_imageCache->setMatchType(path, KGeoTag::MatchType::Set);
    m_imageCache->setChanged(path, true);
    m_assignedImages->addOrUpdateImage(path);
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

    const auto images = m_unAssignedImages->allImages();
    for (const auto &path : images) {
        const auto coordinates = m_mapWidget->findExactCoordinates(m_imageCache->date(path));
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
    const auto files = m_unAssignedImages->allImages();

    QProgressDialog progress(i18n("Assigning images ..."), i18n("Cancel"), 0, files.count(), this);
    progress.setWindowModality(Qt::WindowModal);

    int processed = 0;
    for (const auto &path : files) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        const auto coordinates
            = m_mapWidget->findInterpolatedCoordinates(m_imageCache->date(path));
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

        const auto coordinates = m_imageCache->coordinates(path);
        if (coordinates.isSet) {
            exif.setGPSInfo(0.0, coordinates.lat, coordinates.lon);
        } else {
            exif.removeGPSInfo();
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
}
