// SPDX-FileCopyrightText: 2021-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "MapCenterInfo.h"
#include "SharedObjects.h"
#include "Coordinates.h"
#include "CoordinatesFormatter.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMenu>

MapCenterInfo::MapCenterInfo(SharedObjects *sharedObjects, QMenu *mapCenterMenu, QWidget *parent)
    : QWidget(parent),
      m_formatter(sharedObjects->coordinatesFormatter()),
      m_locale(sharedObjects->locale()),
      m_mapCenterMenu(mapCenterMenu)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_coordinatesLabel = new QLabel;
    m_coordinatesLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_coordinatesLabel, &QLabel::customContextMenuRequested, this, [this](const QPoint &pos)
            {
                m_mapCenterMenu->exec(m_coordinatesLabel->mapToGlobal(pos));
            });
    layout->addWidget(m_coordinatesLabel);

    layout->addStretch();

    m_dateTimeLabel = new QLabel;
    layout->addWidget(m_dateTimeLabel);
}

void MapCenterInfo::displayCoordinates(const Coordinates &coordinates)
{
    m_coordinatesLabel->setText(i18n("Map center position: %1", m_formatter->format(coordinates)));
}

void MapCenterInfo::mapMoved(const Coordinates &center)
{
    displayCoordinates(center);
    m_dateTimeLabel->clear();
}

void MapCenterInfo::trackPointCentered(const Coordinates &coordinates, const QDateTime &dateTime)
{
    displayCoordinates(coordinates);
    m_dateTimeLabel->setText(dateTime.toString(m_locale->dateTimeFormat()));
}
