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
#include "PreviewWidget.h"
#include "ImageCache.h"
#include "ImagePreview.h"
#include "KGeoTag.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QDebug>
#include <QLabel>

PreviewWidget::PreviewWidget(ImageCache *imageCache, QWidget *parent)
    : QWidget(parent), m_imageCache(imageCache)
{
    auto *layout = new QVBoxLayout(this);

    m_path = new QLabel;
    m_path->setWordWrap(true);
    m_path->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    layout->addWidget(m_path);

    m_date = new QLabel;
    m_date->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    layout->addWidget(m_date);

    m_coordinates = new QLabel;
    m_coordinates->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    layout->addWidget(m_coordinates);

    m_preview = new ImagePreview(imageCache);
    layout->addWidget(m_preview);
}

void PreviewWidget::setImage(const QString &path)
{
    m_path->setText(i18n("File: %1", path));
    m_date->setText(i18n("Date: %1",
                         m_imageCache->date(path).toString(i18n("yyyy-MM-dd hh:mm:ss"))));

    const auto coordinates = m_imageCache->coordinates(path);
    if (coordinates.isSet) {
        m_coordinates->setText(i18n("Longitude: %1 / Latitude: %2",
                                    coordinates.lon, coordinates.lat));
    } else {
        m_coordinates->setText(i18n("<i>No coordinates set</i>"));
    }

    m_preview->setImage(path);
}
