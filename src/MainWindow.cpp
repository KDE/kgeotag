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
#include "ImagesWidget.h"
#include "PreviewWidget.h"
#include "MapWidget.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QMenuBar>
#include <QAction>
#include <QDockWidget>
#include <QGuiApplication>
#include <QScreen>

MainWindow::MainWindow() : QMainWindow()
{
    // Menu setup

    auto *fileMenu = menuBar()->addMenu(i18n("File"));
    auto *quitAction = fileMenu->addAction(i18n("Quit"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // Dock setup

    setDockNestingEnabled(true);

    auto *imagesDock = createDockWidget(i18n("Images"), new ImagesWidget,
                                        QStringLiteral("imagesDock"));
    auto *previewDock = createDockWidget(i18n("Preview"), new PreviewWidget,
                                         QStringLiteral("previewDock"));
    createDockWidget(i18n("Map"), new MapWidget, QStringLiteral("mapDock"));

    // Size initialization

    const QRect availableGeometry = QGuiApplication::primaryScreen()->availableGeometry();
    if (availableGeometry.width() > 1024 && availableGeometry.height() > 765) {
        resize(QSize(1024, 765));
    } else {
        setWindowState(Qt::WindowMaximized);
    }

    splitDockWidget(imagesDock, previewDock, Qt::Vertical);
}

QDockWidget *MainWindow::createDockWidget(const QString &title, QWidget *widget,
                                          const QString &objectName)
{
    auto *dock = new QDockWidget(title, this);
    dock->setObjectName(objectName);
    dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    dock->setWidget(widget);
    addDockWidget(Qt::TopDockWidgetArea, dock);
    return dock;
}
