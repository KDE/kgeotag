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
#include "ImagesList.h"
#include "ImageCache.h"

// Qt includes
#include <QIcon>
#include <QDebug>

ImagesList::ImagesList(ImageCache *imageCache, QWidget *parent)
    : QListWidget(parent), m_imageCache(imageCache)
{
    setSortingEnabled(true);
    setIconSize(m_imageCache->thumbnailSize());

    connect(this, &QListWidget::itemClicked, [this](QListWidgetItem *item)
            {
                emit imageSelected(item->data(Qt::UserRole).toString());
            });
}

void ImagesList::addImage(const QString fileName, const QString &path)
{
    auto *item = new QListWidgetItem(QIcon(QPixmap::fromImage(m_imageCache->thumbnail(path))),
                                     fileName);
    item->setData(Qt::UserRole, path);
    addItem(item);
}