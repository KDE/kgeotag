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

ImageCache::ImageCache(QObject *parent) : QObject(parent)
{
}

bool ImageCache::addImage(const QString &path)
{
    if (m_thumbnails.contains(path)) {
        return false;
    }

    const QImage image = QImage(path);
    if (image.isNull()) {
        return false;
    }

    m_thumbnails.insert(path, image.scaled(QSize(32, 32), Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
    m_previews.insert(path, image.scaled(QSize(400, 400), Qt::KeepAspectRatio));
    return true;
}

QImage ImageCache::thumbnail(const QString &path) const
{
    return m_thumbnails.value(path);
}

QImage ImageCache::preview(const QString &path) const
{
    return m_previews.value(path);
}
