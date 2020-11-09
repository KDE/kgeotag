/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>
    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "ImagePreview.h"
#include "SharedObjects.h"

// Qt includes
#include <QTimer>
#include <QModelIndex>

ImagePreview::ImagePreview(QWidget *parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setMinimumSize(128, 128);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    m_smoothTimer = new QTimer(this);
    m_smoothTimer->setSingleShot(true);
    connect(m_smoothTimer, &QTimer::timeout, this, &ImagePreview::setScaledPreview);
}

void ImagePreview::setImage(const QModelIndex &index)
{
    m_currentImage = index.isValid() ? index.data(KGeoTag::PreviewRole).value<QImage>() : QImage();
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
    if (m_currentImage.isNull()) {
        setPixmap(QPixmap());
        return;
    }

    const QImage scaledImage = m_currentImage.scaled(size(), Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation);
    setPixmap(QPixmap::fromImage(scaledImage));
}
