/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Local includes
#include "KGeoTag.h"
#include "ElevationEngine.h"
#include "Coordinates.h"

// Qt includes
#include <QMainWindow>

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
    void addGpx(const QVector<QString> &paths);
    void addImages(const QVector<QString> &paths);
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

private: // Functions
    QDockWidget *createImagesDock(KGeoTag::ImagesListType type, const QString &title,
                                  const QString &dockId);
    QDockWidget *createDockWidget(const QString &title, QWidget *widget, const QString &objectName);
    void lookupElevation(const QVector<QString> &paths);

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
    AutomaticMatchingWidget *m_automaticMatchingWidget;

    QDockWidget *m_previewDock;
    QDockWidget *m_fixDriftDock;
    QDockWidget *m_automaticMatchingDock;
    QDockWidget *m_bookmarksDock;
    QDockWidget *m_mapDock;
    QDockWidget *m_unAssignedImagesDock;
    QDockWidget *m_assignedOrAllImagesDock;

};

#endif // MAINWINDOW_H
