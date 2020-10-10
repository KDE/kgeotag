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
#include "DragableImagesList.h"
#include "PreviewWidget.h"
#include "MapWidget.h"
#include "Coordinates.h"

// KDE includes
#include <KLocalizedString>

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

    auto *assignImagesAction = fileMenu->addAction(i18n("Assign images"));
    connect(assignImagesAction, &QAction::triggered, this, &MainWindow::assignImages);

    fileMenu->addSeparator();

    auto *quitAction = fileMenu->addAction(i18n("Quit"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // Dock setup
    setDockNestingEnabled(true);

    // Unassigned images
    m_unAssignedImages = new DragableImagesList(m_imageCache);
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
    connect(m_mapWidget, &MapWidget::imageAssigned, this, &MainWindow::imageAssigned);

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
        progress.setValue(processed);
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
            m_unAssignedImages->addImage(info.fileName(), canonicalPath);
        } else {
            m_mapWidget->addImage(canonicalPath, coordinates.lon, coordinates.lat);
            m_assignedImages->addImage(info.fileName(), canonicalPath);
        }

        processed++;
    }

    QApplication::restoreOverrideCursor();
}

void MainWindow::imageAssigned(const QString &path)
{
    const QFileInfo info(path);
    m_unAssignedImages->removeCurrentImage();
    m_assignedImages->addImage(info.fileName(), path);
}

void MainWindow::assignImages()
{
    const auto images = m_unAssignedImages->allImages();
    for (const auto &image : images) {
        const auto coordinates = m_mapWidget->findCoordinates(m_imageCache->date(image));
        if (coordinates.isSet) {
            m_imageCache->setCoordinates(image, coordinates);
            m_unAssignedImages->removeImage(image);
            m_mapWidget->addImage(image, coordinates);
            const QFileInfo info(image);
            m_assignedImages->addImage(info.fileName(), image);
        }
    }
}
