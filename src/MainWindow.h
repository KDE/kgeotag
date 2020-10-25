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

// Qt includes
#include <QMainWindow>

// Local classes
class SharedObjects;
class Settings;
class ImageCache;
class GpxEngine;
class ElevationEngine;
class ImagesList;
class PreviewWidget;
class MapWidget;
class FixDriftWidget;
class BookmarksWidget;

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
    void addGpx();
    void addImages();
    void imageDropped(const QString &path);
    void assignExactMatches();
    void assignInterpolatedMatches();
    void saveChanges();
    void showSettings();
    void removeCoordinates(const QVector<QString> &paths);
    void discardChanges(const QVector<QString> &paths);
    void assignToMapCenter(const QVector<QString> &paths);
    void assignTo(const QVector<QString> &paths, const KGeoTag::Coordinates &coordinates);
    void checkUpdatePreview(const QVector<QString> &paths);
    void elevationLookupFailed(const QString &errorMessage);

private: // Functions
    QDockWidget *createDockWidget(const QString &title, QWidget *widget, const QString &objectName);
    void assignImage(const QString &path, const KGeoTag::Coordinates &coordinates);

private: // Variables
    Settings *m_settings;
    ImageCache *m_imageCache;
    GpxEngine *m_gpxEngine;
    ImagesList *m_assignedImages;
    ImagesList *m_unAssignedImages;
    PreviewWidget *m_previewWidget;
    MapWidget *m_mapWidget;
    FixDriftWidget *m_fixDriftWidget;
    BookmarksWidget *m_bookmarksWidget;

};

#endif // MAINWINDOW_H
