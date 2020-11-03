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

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QObject>
#include <QHash>
#include <QImage>
#include <QDateTime>

// Local classes
class Settings;

// exiv2 classes
namespace Exiv2
{
class ExifData;
}

class ImageCache : public QObject
{
    Q_OBJECT

public:
    explicit ImageCache(QObject *parent, Settings *settings);
    bool addImage(const QString &path);

private: // Structs
    struct ImageData
    {
        QImage thumbnail;
        QImage preview;
        QDateTime date;
        KGeoTag::Coordinates originalCoordinates = KGeoTag::NoCoordinates;
        KGeoTag::Coordinates coordinates = KGeoTag::NoCoordinates;
    };

private: // Variables
    Settings *m_settings;
    QHash<QString, ImageData> m_imageData;

};

#endif // IMAGECACHE_H
