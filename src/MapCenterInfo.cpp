/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "MapCenterInfo.h"
#include "Coordinates.h"
#include "CoordinatesFormatter.h"

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
    layout->setContentsMargins(0, 0, 0, 0);

    m_label = new QLabel;
    layout->addWidget(m_label);
}

void MapCenterInfo::mapMoved(const Coordinates &center)
{
    m_label->setText(i18n("Map center position: %1, %2",
                          m_formatter->lon(center),
                          m_formatter->lat(center)));
}
