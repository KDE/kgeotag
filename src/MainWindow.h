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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Local includes
#include "KGeoTag.h"
#include "ElevationEngine.h"

// Qt includes
#include <QMainWindow>

// Local classes
class SharedObjects;
class Settings;
class ImageCache;
class GpxEngine;
class PreviewWidget;
class MapWidget;
class FixDriftWidget;
class BookmarksWidget;
class ImagesModel;
class ImagesListView;

// Qt classes
class QDockWidget;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SharedObjects *sharedObjects);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private slots:
    void setDefaultDockArrangement();
    void addGpx();
    void addImages();
    void imagesDropped(const QVector<QString> &paths);
    void saveChanges();
    void showSettings();
    void assignTo(const QVector<QString> &paths, const KGeoTag::Coordinates &coordinates);
    void checkUpdatePreview(const QVector<QString> &paths);
    void elevationProcessed(ElevationEngine::Target target, const QVector<QString> &paths,
                            const QVector<double> &elevations);
    void elevationLookupFailed(const QString &errorMessage);
    void notAllElevationsPresent(int locationsCount, int elevationsCount);

    void searchExactMatches(ImagesListView *list);
    void searchInterpolatedMatches(ImagesListView *list);
    void assignToMapCenter(ImagesListView *list);
    void assignManually(ImagesListView *list);
    void editCoordinates(ImagesListView *list);
    void removeCoordinates(ImagesListView *list);
    void discardChanges(ImagesListView *list);
    void lookupElevation(ImagesListView *list);

private: // Functions
    QDockWidget *createImagesDock(const QString &title, const QString &dockId);
    QDockWidget *createDockWidget(const QString &title, QWidget *widget, const QString &objectName);
    void assignImage(const QString &path, const KGeoTag::Coordinates &coordinates);
    void lookupElevation(const QVector<QString> &paths);

private: // Variables
    SharedObjects *m_sharedObjects;
    Settings *m_settings;
    ImageCache *m_imageCache;
    GpxEngine *m_gpxEngine;
    ElevationEngine *m_elevationEngine;
    PreviewWidget *m_previewWidget;
    MapWidget *m_mapWidget;
    FixDriftWidget *m_fixDriftWidget;
    BookmarksWidget *m_bookmarksWidget;
    ImagesModel *m_imagesModel;

    QDockWidget *m_previewDock;
    QDockWidget *m_fixDriftDock;
    QDockWidget *m_bookmarksDock;
    QDockWidget *m_mapDock;
    QDockWidget *m_unAssignedImagesDock;
    QDockWidget *m_assignedImagesDock;

};

#endif // MAINWINDOW_H
