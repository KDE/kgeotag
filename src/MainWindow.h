/* SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Local includes
#include "KGeoTag.h"
#include "ElevationEngine.h"
#include "Coordinates.h"

// KDE includes
#include <KXmlGuiWindow>

// Local classes
class SharedObjects;
class Settings;
class GpxEngine;
class PreviewWidget;
class MapWidget;
class FixDriftWidget;
class BookmarksWidget;
class ImagesModel;
class ImagesListView;
class AutomaticMatchingWidget;
class TracksListView;
class GeoDataModel;
class MapCenterInfo;

// Qt classes
class QDockWidget;
class QCloseEvent;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(SharedObjects *sharedObjects);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void updateImagesListsMode();
    void setDefaultDockArrangement();

    void addFiles();
    void addDirectory();

    void imagesDropped(const QVector<QString> &paths);
    void saveChanges();
    void showSettings();
    void assignTo(const QVector<QString> &paths, const Coordinates &coordinates);
    void checkUpdatePreview(const QVector<QString> &paths);
    void elevationProcessed(ElevationEngine::Target target, const QVector<QString> &paths,
                            const QVector<double> &elevations);
    void elevationLookupFailed(const QString &errorMessage);
    void notAllElevationsPresent(int locationsCount, int elevationsCount);
    void triggerAutomaticMatching(ImagesListView *list, KGeoTag::SearchType searchType);
    void triggerCompleteAutomaticMatching(KGeoTag::SearchType searchType);
    void matchAutomatically(const QVector<QString> &paths, KGeoTag::SearchType searchType);
    void assignToMapCenter(ImagesListView *list);
    void assignManually(ImagesListView *list);
    void editCoordinates(ImagesListView *list);
    void removeCoordinates(ImagesListView *list);
    void removeCoordinates(const QVector<QString> &paths);
    void discardChanges(ImagesListView *list);
    void lookupElevation(ImagesListView *list);
    void imagesTimeZoneChanged();
    void cameraDriftSettingsChanged();
    void centerTrackPoint(int trackIndex, int trackPointIndex);

    void removeImages(ImagesListView *list);
    void removeProcessedSavedImages();
    void removeAllImages();
    void removeTracks();
    void removeAllTracks();
    void removeEverything();

private: // Functions
    QDockWidget *createImagesDock(KGeoTag::ImagesListType type, const QString &title,
                                  const QString &dockId);
    QDockWidget *createDockWidget(const QString &title, QWidget *widget, const QString &objectName);
    void addGpx(const QVector<QString> &paths);
    void addImages(const QVector<QString> &paths);
    void lookupElevation(const QVector<QString> &paths);
    QString saveFailedHeader(int processed, int allImages) const;
    QString skipRetryCancelText(int processed, int allImages) const;
    bool checkForPendingChanges();

private: // Variables
    SharedObjects *m_sharedObjects;
    Settings *m_settings;
    GpxEngine *m_gpxEngine;
    ElevationEngine *m_elevationEngine;
    PreviewWidget *m_previewWidget;
    MapWidget *m_mapWidget;
    FixDriftWidget *m_fixDriftWidget;
    BookmarksWidget *m_bookmarksWidget;
    ImagesModel *m_imagesModel;
    GeoDataModel *m_geoDataModel;
    AutomaticMatchingWidget *m_automaticMatchingWidget;
    TracksListView *m_tracksView;
    MapCenterInfo *m_mapCenterInfo;

    QDockWidget *m_previewDock;
    QDockWidget *m_fixDriftDock;
    QDockWidget *m_automaticMatchingDock;
    QDockWidget *m_bookmarksDock;
    QDockWidget *m_mapDock;
    QDockWidget *m_unAssignedImagesDock;
    QDockWidget *m_assignedOrAllImagesDock;
    QDockWidget *m_tracksDock;

};

#endif // MAINWINDOW_H
