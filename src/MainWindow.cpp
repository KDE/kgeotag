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

// KDE includes
#include <KLocalizedString>
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

MainWindow::MainWindow() : QMainWindow()
{
    m_settings = new Settings(this);
    m_imageCache = new ImageCache(this, m_settings);

    // Menu setup

    auto *fileMenu = menuBar()->addMenu(i18n("File"));

    auto *addGpxAction = fileMenu->addAction(i18n("Add GPX track"));
    connect(addGpxAction, &QAction::triggered, this, &MainWindow::addGpx);

    auto *addImagesAction = fileMenu->addAction(i18n("Add images"));
    connect(addImagesAction, &QAction::triggered, this, &MainWindow::addImages);

    fileMenu->addSeparator();

    auto *assignExactMatchesAction = fileMenu->addAction(i18n("Assign images (exact matches)"));
    connect(assignExactMatchesAction, &QAction::triggered, this, &MainWindow::assignExactMatches);

    auto *assignInterpolatedMatchesAction = fileMenu->addAction(i18n("Assign images "
                                                                     "(interpolated)"));
    connect(assignInterpolatedMatchesAction, &QAction::triggered,
            this, &MainWindow::assignInterpolatedMatches);

    fileMenu->addSeparator();

    auto *saveChangesAction = fileMenu->addAction(i18n("Save changed images"));
    connect(saveChangesAction, &QAction::triggered, this, &MainWindow::saveChanges);

    fileMenu->addSeparator();

    auto *quitAction = fileMenu->addAction(i18n("Quit"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // Dock setup
    setDockNestingEnabled(true);

    // Unassigned images
    m_unAssignedImages = new ImagesList(m_imageCache);
    auto *unassignedImagesDock = createDockWidget(i18n("Unassigned images"), m_unAssignedImages,
                                                  QStringLiteral("unassignedImagesDock"));

    // Assigned images
    m_assignedImages = new ImagesList(m_imageCache);
    auto *assignedImagesDock = createDockWidget(i18n("Assigned images"), m_assignedImages,
                                                QStringLiteral("assignedImagesDock"));

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
    connect(m_assignedImages, &ImagesList::imageSelected, m_mapWidget, &MapWidget::centerImage);
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
    QApplication::setOverrideCursor(Qt::WaitCursor);
    const auto &files = m_imageCache->changedImages();

    QProgressDialog progress(i18n("Saving changes ..."), i18n("Cancel"), 0, files.count(), this);
    progress.setWindowModality(Qt::WindowModal);

    int processed = 0;
    for (const QString &path : files) {
        progress.setValue(processed++);
        if (progress.wasCanceled()) {
            break;
        }

        // Create a backup of the file
        if (! QFile::copy(path, path + QStringLiteral(".orig"))) {
            progress.deleteLater();
            QApplication::restoreOverrideCursor();
            QTimer::singleShot(0, [this, path]
            {
                QMessageBox::warning(this,
                    i18n("Save changes"),
                    i18n("Failed to create a backup of \"%1\". No changes will be written to it. "
                         "Aborting.", path));
            });
            return;
        }

        // Write the GPS information

        auto exif = KExiv2Iface::KExiv2(path);
        const auto coordinates = m_imageCache->coordinates(path);
        exif.setGPSInfo(0.0, coordinates.lat, coordinates.lon);

        if (! exif.save(path)) {
            progress.deleteLater();
            QApplication::restoreOverrideCursor();
            QTimer::singleShot(0, [this, path]
            {
                QMessageBox::warning(this,
                    i18n("Save changes"),
                    i18n("Could not save EXIF information to \"%1\". Aborting.", path));
            });
            return;
        }

        m_imageCache->setChanged(path, false);
        m_assignedImages->addOrUpdateImage(path);
    }

    QApplication::restoreOverrideCursor();
}
