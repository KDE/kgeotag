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

// Local includes
#include "ImageCache.h"
#include "Settings.h"

// KDE includes
#include <KExiv2/KExiv2>

// Qt includes
#include <QDebug>
#include <QTransform>

ImageCache::ImageCache(QObject *parent, Settings *settings)
    : QObject(parent), m_settings(settings)
{
}

bool ImageCache::addImage(const QString &path)
{
    if (m_imageData.contains(path)) {
        return true;
    }

    QImage image = QImage(path);
    if (image.isNull()) {
        return false;
    }

    ImageData data;

    // Read the exif data
    auto exif = KExiv2Iface::KExiv2();
    exif.setUseXMPSidecar4Reading(true);
    if (! exif.load(path)) {
        return false;
    }

    // Read the date (falling back to the file's date if nothing is set)
    data.date = exif.getImageDateTime();

    // Try to read gps information
    double altitude;
    double latitude;
    double longitude;
    if (exif.getGPSInfo(altitude, latitude, longitude)) {
        data.originalCoordinates = KGeoTag::Coordinates { longitude, latitude, altitude, true };
        data.coordinates = KGeoTag::Coordinates { longitude, latitude, altitude, true };
    }

    // Fix the image's orientation
    exif.rotateExifQImage(image, exif.getImageOrientation());

    // Create a smaller thumbnail
    data.thumbnail = image.scaled(m_settings->thumbnailSize(), Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);

    // Create a bigger preview (to be scaled according to the view size)
    data.preview = image.scaled(m_settings->previewSize(), Qt::KeepAspectRatio);

    m_imageData.insert(path, data);
    return true;
}

bool ImageCache::contains(const QString &path) const
{
    return m_imageData.contains(path);
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

KGeoTag::Coordinates ImageCache::coordinates(const QString &path) const
{
    return m_imageData[path].coordinates;
}

void ImageCache::setCoordinates(const QString &path, double lon, double lat, double alt)
{
    setCoordinates(path, KGeoTag::Coordinates { lon, lat, alt, true });
}

void ImageCache::setCoordinates(const QString &path, const KGeoTag::Coordinates &coordinates)
{
    m_imageData[path].coordinates = coordinates;
}

void ImageCache::setChanged(const QString &path, bool changed)
{
    if (m_imageData.contains(path)) {
        m_imageData[path].changed = changed;
    }
}

QVector<QString> ImageCache::changedImages() const
{
    QVector<QString> changed;
    QHashIterator<QString, ImageData> it(m_imageData);
    while (it.hasNext()) {
        it.next();
        if (it.value().changed) {
            changed.append(it.key());
        }
    }
    return changed;
}

void ImageCache::setMatchType(const QString &path, KGeoTag::MatchType matchType)
{
    m_imageData[path].matchType = matchType;
}

KGeoTag::MatchType ImageCache::matchType(const QString &path)
{
    return m_imageData[path].matchType;
}

bool ImageCache::changed(const QString &path) const
{
    return m_imageData[path].changed;
}

void ImageCache::resetChanges(const QString &path)
{
    auto &data = m_imageData[path];
    data.coordinates = data.originalCoordinates;
    data.matchType = KGeoTag::MatchType::None;
    data.changed = false;
}
