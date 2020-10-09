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

#ifndef IMAGECACHE_H
#define IMAGECACHE_H

// Local includes
#include "Coordinates.h"

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
    bool contains(const QString &path) const;
    QImage thumbnail(const QString &path) const;
    QImage preview(const QString &path) const;
    QDateTime date(const QString &path) const;
    QSize thumbnailSize() const;
    Coordinates::Data coordinates(const QString &path) const;
    void setCoordinates(const QString &path, double lon, double lat);

private: // Functions
    QString getExifValue(Exiv2::ExifData &data, const char *key) const;
    double parseExifLonLat(const QString &lonLat, const QString &ref) const;

private: // Structs
    struct ImageData {
        QImage thumbnail;
        QImage preview;
        QDateTime date;
        Coordinates::Data coordinates;
    };

private: // Variables
    Settings *m_settings;
    QHash<QString, ImageData> m_imageData;

};

#endif // IMAGECACHE_H
