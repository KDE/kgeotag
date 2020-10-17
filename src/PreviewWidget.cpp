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
#include <QLocale>
#include <QGridLayout>

// C++ includes
#include <cmath>

PreviewWidget::PreviewWidget(ImageCache *imageCache, QWidget *parent)
    : QWidget(parent), m_imageCache(imageCache)
{
    auto *layout = new QVBoxLayout(this);

    auto *infoLayout = new QGridLayout;
    layout->addLayout(infoLayout);

    auto *pathLabel = new QLabel(i18n("Image:"));
    pathLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    infoLayout->addWidget(pathLabel, 0, 0);

    m_path = new QLabel;
    m_path->setWordWrap(true);
    m_path->setTextInteractionFlags(Qt::TextSelectableByMouse);
    infoLayout->addWidget(m_path, 0, 1);

    auto *dateTimeLabel = new QLabel(i18n("Date/Time:"));
    dateTimeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    infoLayout->addWidget(dateTimeLabel, 1, 0);

    m_date = new QLabel;
    m_date->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_date->setWordWrap(true);
    infoLayout->addWidget(m_date, 1, 1);

    auto *coordinatesLabel = new QLabel(i18n("Coordinates:"));
    coordinatesLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    infoLayout->addWidget(coordinatesLabel, 2, 0);

    m_coordinates = new QLabel;
    m_coordinates->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_coordinates->setWordWrap(true);
    infoLayout->addWidget(m_coordinates, 2, 1);

    m_preview = new ImagePreview(imageCache);
    layout->addWidget(m_preview);

    m_matchString[KGeoTag::MatchType::None] = i18n("read from file");
    m_matchString[KGeoTag::MatchType::Exact] = i18n("exact match");
    m_matchString[KGeoTag::MatchType::Interpolated] = i18n("interpolated match");
    m_matchString[KGeoTag::MatchType::Set] = i18n("manually set");
}

void PreviewWidget::setImage(const QString &path)
{
    m_currentImage = path;

    if (path.isEmpty()) {
        m_path->setText(path);
        m_date->setText(path);
        m_coordinates->setText(path);
        m_preview->setImage(path);
        return;
    }

    m_path->setText(path);
    QLocale locale;
    m_date->setText(m_imageCache->date(path).toString(locale.dateTimeFormat()));

    const auto coordinates = m_imageCache->coordinates(path);
    if (coordinates.isSet) {
        m_coordinates->setText(i18n("<p>Position: %1° %2, %3° %4; Altitude: %5 m<br/>(%6)</p>",
            std::abs(coordinates.lon), coordinates.lon >= 0 ? i18nc("Cardinal direction", "E")
                                                            : i18nc("Cardinal direction", "W"),
            std::abs(coordinates.lat), coordinates.lat >= 0 ? i18nc("Cardinal direction", "N")
                                                            : i18nc("Cardinal direction", "S"),
            coordinates.alt,
            m_matchString.value(m_imageCache->matchType(path))));
    } else {
        m_coordinates->setText(i18n("<i>No coordinates set</i>"));
    }

    m_preview->setImage(path);
}

QString PreviewWidget::currentImage() const
{
    return m_currentImage;
}
