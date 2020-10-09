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
#include "DragableImagesList.h"

// Qt includes
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>

DragableImagesList::DragableImagesList(ImageCache *imageCache, QWidget *parent)
    : ImagesList(imageCache, parent)
{
}

void DragableImagesList::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }

    QListWidget::mousePressEvent(event);
}

void DragableImagesList::mouseMoveEvent(QMouseEvent *event)
{
    const auto *item = currentItem();

    if (! (event->buttons() & Qt::LeftButton)
        || item == nullptr
        || (event->pos() - m_dragStartPosition).manhattanLength()
           < QApplication::startDragDistance()) {

        return;
    }

    auto *drag = new QDrag(this);
    drag->setPixmap(item->icon().pixmap(iconSize()));

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(item->data(Qt::UserRole).toString());
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}
