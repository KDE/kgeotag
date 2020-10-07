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
#include "MapWidget.h"

// Marble includes
#include <marble/MarbleWidget.h>
#include <marble/AbstractFloatItem.h>

// Qt includes
#include <QVBoxLayout>
#include <QDebug>

MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    m_mapWidget = new Marble::MarbleWidget;
    m_mapWidget->setProjection(Marble::Mercator);
    m_mapWidget->setMapThemeId(QStringLiteral("earth/openstreetmap/openstreetmap.dgml"));

    layout->addWidget(m_mapWidget);
    m_mapWidget->show();
}

QHash<QString, bool> MapWidget::floatersVisibility() const
{
    QHash<QString, bool> visibility;

    const auto floatItems = m_mapWidget->floatItems();
    for (const auto &item : floatItems) {
        visibility.insert(item->name(), item->visible());
    }

    return visibility;
}

void MapWidget::setFloatersVisibility(const QHash<QString, bool> &data)
{
    const auto floatItems = m_mapWidget->floatItems();
    for (const auto &item : floatItems) {
        const auto name = item->name();
        if (data.contains(name)) {
            item->setVisible(data.value(name));
        }
    }
}
