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
#include "ImageCache.h"
#include "Settings.h"

// exiv2 includes
#include <exiv2/exiv2.hpp>

// Qt includes
#include <QDebug>

ImageCache::ImageCache(QObject *parent, Settings *settings)
    : QObject(parent), m_settings(settings)
{
}

bool ImageCache::addImage(const QString &path)
{
    if (m_imageData.contains(path)) {
        return false;
    }

    const QImage image = QImage(path);
    if (image.isNull()) {
        return false;
    }

    auto exif = Exiv2::ImageFactory::open(path.toLocal8Bit().data());
    exif->readMetadata();
    auto &exifData = exif->exifData();

    const auto date = QDateTime::fromString(getExifValue(exifData, "Exif.Photo.DateTimeOriginal"),
                                            QStringLiteral("yyyy:MM:dd hh:mm:ss"));
    const auto coordinates = getExifGps(exifData);

    const QImage thumbnail = image.scaled(m_settings->thumbnailSize(), Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
    const QImage preview = image.scaled(m_settings->previewSize(), Qt::KeepAspectRatio);

    m_imageData.insert(path, ImageData { thumbnail, preview, date, coordinates });

    return true;
}

Coordinates::Data ImageCache::getExifGps(Exiv2::ExifData &data) const
{
    if (getExifValue(data, "Exif.GPSInfo.GPSVersionID").isEmpty()) {
        return Coordinates::Data { 0.0, 0.0, false };
    }

    const QString longitude    = getExifValue(data, "Exif.GPSInfo.GPSLongitude");
    const QString longitudeRef = getExifValue(data, "Exif.GPSInfo.GPSLongitudeRef");
    const QString latitude     = getExifValue(data, "Exif.GPSInfo.GPSLatitude");
    const QString latitudeRef  = getExifValue(data, "Exif.GPSInfo.GPSLatitudeRef");

    if (longitude.isEmpty() || longitudeRef.isEmpty()
        || latitude.isEmpty() || latitudeRef.isEmpty()
        || longitude.count(QStringLiteral(" ")) != 2
        || longitude.count(QStringLiteral("/")) != 3
        || latitude.count(QStringLiteral(" ")) != 2
        || latitude.count(QStringLiteral("/")) != 3) {

        return Coordinates::Data { 0.0, 0.0, false };
    }

    return Coordinates::Data { parseExifLonLat(longitude, longitudeRef),
                               parseExifLonLat(latitude, latitudeRef),
                               true };
}

double ImageCache::parseExifLonLat(const QString &lonLat, const QString &ref) const
{
    QVector<double> parsedLonLat;
    const auto degMinSec = lonLat.split(QLatin1String(" "));
    for (const auto &part : degMinSec) {
        const auto fraction = part.split(QLatin1String("/"));
        parsedLonLat.append(fraction.at(0).toDouble() / fraction.at(1).toDouble());
    }

    return (parsedLonLat.at(0)
            + parsedLonLat.at(1) / 60.0
            + parsedLonLat.at(2) / 3600.0)
           * ((ref == QStringLiteral("N") || ref == QStringLiteral("E")) ? 1 : -1);
}

bool ImageCache::contains(const QString &path) const
{
    return m_imageData.contains(path);
}

QString ImageCache::getExifValue(Exiv2::ExifData &data, const char *key) const
{
    auto it = data.findKey(Exiv2::ExifKey(key));
    if (it != data.end()) {
        return QString::fromStdString(it->toString());
    } else {
        return QString();
    }
}

QImage ImageCache::thumbnail(const QString &path) const
{
    return m_imageData[path].thumbnail;
}

QImage ImageCache::preview(const QString &path) const
{
    return m_imageData[path].preview;
}

QDateTime ImageCache::date(const QString &path) const
{
    return m_imageData[path].date;
}

QSize ImageCache::thumbnailSize() const
{
    return m_settings->thumbnailSize();
}

Coordinates::Data ImageCache::coordinates(const QString &path) const
{
    return m_imageData[path].coordinates;
}

void ImageCache::setCoordinates(const QString &path, double lon, double lat)
{
    m_imageData[path].coordinates = Coordinates::Data { lon, lat, true };
}
