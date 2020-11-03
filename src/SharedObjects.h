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

#ifndef SHAREDOBJECTS_H
#define SHAREDOBJECTS_H

// Local includes
#include "KGeoTag.h"

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

    void setBookmarks(const QHash<QString, KGeoTag::Coordinates> *bookmarks);
    const QHash<QString, KGeoTag::Coordinates> *bookmarks() const;

private: // Variables
    Settings *m_settings;
    ImagesModel *m_imagesModel;
    GpxEngine *m_gpxEngine;
    ElevationEngine *m_elevationEngine;
    MapWidget *m_mapWidget;
    QLocale m_locale;
    CoordinatesFormatter *m_coordinatesFormatter;

    const QHash<QString, KGeoTag::Coordinates> *m_bookmarks;

};

#endif // SHAREDOBJECTS_H
