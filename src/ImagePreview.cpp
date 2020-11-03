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
#include "ImagePreview.h"
#include "SharedObjects.h"
#include "ImagesModel.h"
#include "ImageCache.h"

// Qt includes
#include <QTimer>

ImagePreview::ImagePreview(SharedObjects *sharedObjects, QWidget *parent)
    : QLabel(parent),
      m_imageCache(sharedObjects->imageCache()),
      m_imagesModel(sharedObjects->imagesModel())
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(128, 128);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    m_smoothTimer = new QTimer(this);
    m_smoothTimer->setSingleShot(true);
    connect(m_smoothTimer, &QTimer::timeout, this, &ImagePreview::setScaledPreview);
}

void ImagePreview::setImage(const QString &path)
{
    m_currentImage = ! path.isEmpty() ? m_imagesModel->preview(path) : QImage();
    setScaledPreview();
}

void ImagePreview::resizeEvent(QResizeEvent *event)
{
    // This happen when no image is set yet
    if (m_currentImage.isNull()) {
        return;
    }

    // During resizing, we use a quickly scaled image
    const QImage scaledImage = m_currentImage.scaled(size(), Qt::KeepAspectRatio);
    setPixmap(QPixmap::fromImage(scaledImage));

    // (Re)start the timer that sets a smoothly scaled version of the image
    m_smoothTimer->start(200);

    QLabel::resizeEvent(event);
}

void ImagePreview::setScaledPreview()
{
    const QImage scaledImage = m_currentImage.scaled(size(), Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation);
    setPixmap(QPixmap::fromImage(scaledImage));
}
