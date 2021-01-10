/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "MapCenterInfo.h"
#include "Coordinates.h"
#include "CoordinatesFormatter.h"

// Marble includes
#include <marble/GeoDataLatLonAltBox.h>

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>

MapCenterInfo::MapCenterInfo(CoordinatesFormatter *formatter, QWidget *parent)
    : QWidget(parent),
      m_formatter(formatter)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto *layout = new QHBoxLayout(this);
    m_label = new QLabel;
    layout->addWidget(m_label);
}

void MapCenterInfo::mapMoved(const Marble::GeoDataLatLonAltBox &visibleBox)
{
    const auto marbleCenter = visibleBox.center();
    const auto center = Coordinates(marbleCenter.longitude(Marble::GeoDataCoordinates::Degree),
                                    marbleCenter.latitude(Marble::GeoDataCoordinates::Degree),
                                    0.0, true);
    m_label->setText(i18n("Map center position: %1, %2",
                          m_formatter->lon(center),
                          m_formatter->lat(center)));
}
