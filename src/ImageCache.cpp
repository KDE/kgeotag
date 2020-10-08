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

    const QImage thumbnail = image.scaled(m_settings->thumbnailSize(), Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
    const QImage preview = image.scaled(m_settings->previewSize(), Qt::KeepAspectRatio);

    m_imageData.insert(path, ImageData { thumbnail, preview, date });

    return true;
}

bool ImageCache::contains(const QString &path) const
{
    return m_imageData.contains(path);
}

QString ImageCache::getExifValue(Exiv2::ExifData &data, const char *key) const
{
    return QString::fromStdString(data[key].value().toString());
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
