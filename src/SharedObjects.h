/* SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SHAREDOBJECTS_H
#define SHAREDOBJECTS_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// Qt includes
#include <QObject>
#include <QLocale>

// Local classes
class Settings;
class ImagesModel;
class GpxEngine;
class ElevationEngine;
class MapWidget;
class CoordinatesFormatter;

class SharedObjects : public QObject
{
    Q_OBJECT

public:
    explicit SharedObjects(QObject *parent = nullptr);
    Settings *settings() const;
    ImagesModel *imagesModel() const;
    GpxEngine *gpxEngine() const;
    ElevationEngine *elevationEngine() const;
    MapWidget *mapWidget() const;
    CoordinatesFormatter *coordinatesFormatter() const;

    void setBookmarks(const QHash<QString, Coordinates> *bookmarks);
    const QHash<QString, Coordinates> *bookmarks() const;

    const QLocale *locale() const;

private: // Variables
    Settings *m_settings;
    ImagesModel *m_imagesModel;
    GpxEngine *m_gpxEngine;
    ElevationEngine *m_elevationEngine;
    MapWidget *m_mapWidget;
    QLocale m_locale;
    CoordinatesFormatter *m_coordinatesFormatter;

    const QHash<QString, Coordinates> *m_bookmarks;

};

#endif // SHAREDOBJECTS_H
