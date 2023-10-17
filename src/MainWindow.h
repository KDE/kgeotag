// SPDX-FileCopyrightText: 2020-2023 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
    void addPathsFromCommandLine(QStringList &paths);

protected:
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void updateImagesListsMode();
    void setDefaultDockArrangement();

    void addFiles(const QStringList &files);
    void addDirectory(const QString &path);
    void addGpx(const QVector<QString> &paths);
    void addImages(const QVector<QString> &paths);

    void imagesDropped(const QVector<QString> &paths);
    void saveSelection(ImagesListView *list);
    void saveAllChanges();
    void showSettings();
    void assignTo(const QVector<QString> &paths, const Coordinates &coordinates);
    void failedToParseClipboard();
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
    void findClosestTrackPoint(const QString &path);
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
    void removeImagesLoadedTagged();
    void removeAllImages();
    void removeTracks();
    void removeAllTracks();
    void removeEverything();

private: // Functions
    QDockWidget *createImagesDock(KGeoTag::ImagesListType type, const QString &title,
                                  const QString &dockId);
    QDockWidget *createDockWidget(const QString &title, QWidget *widget, const QString &objectName);
    void lookupElevation(const QVector<QString> &paths);
    QString saveFailedHeader(int processed, int allImages) const;
    QString skipRetryCancelText(int processed, int allImages) const;
    bool checkForPendingChanges();
    void saveChanges(const QVector<QString> &files);

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
