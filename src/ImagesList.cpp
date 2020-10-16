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

// KDE includes
#include <KLocalizedString>
#include <KColorScheme>

// Qt includes
#include <QIcon>
#include <QDebug>
#include <QApplication>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QFileInfo>
#include <QMenu>
#include <QAction>

// C++ includes
#include <functional>

ImagesList::ImagesList(ImageCache *imageCache, QWidget *parent)
    : QListWidget(parent), m_imageCache(imageCache)
{
    setSortingEnabled(true);
    setIconSize(m_imageCache->thumbnailSize());

    connect(this, &QListWidget::currentItemChanged, this, &ImagesList::imageHighlighted);
    connect(this, &QListWidget::itemClicked,
            this, std::bind(&ImagesList::imageHighlighted, this, std::placeholders::_1, nullptr));

    m_contextMenu = new QMenu(this);

    m_removeCoordinates = m_contextMenu->addAction(i18n("Remove coordinates"));
    connect(m_removeCoordinates, &QAction::triggered, [this]
            { emit removeCoordinates(currentItem()->data(Qt::UserRole).toString()); });

    m_discardChanges = m_contextMenu->addAction(i18n("Discard changes"));
    connect(m_discardChanges, &QAction::triggered, [this]
            { emit discardChanges(currentItem()->data(Qt::UserRole).toString()); });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QListWidget::customContextMenuRequested, this, &ImagesList::showContextMenu);
}

void ImagesList::imageHighlighted(QListWidgetItem *item, QListWidgetItem *) const
{
    if (item != nullptr) {
        emit imageSelected(item->data(Qt::UserRole).toString());
    }
}

void ImagesList::addOrUpdateImage(const QString &path)
{
    bool itemFound = false;
    QListWidgetItem *imageItem;

    // Check for an existing entry we want to update
    for (int i = 0; i < count(); i++) {
        if (item(i)->data(Qt::UserRole).toString() == path) {
            imageItem = item(i);
            itemFound = true;
            break;
        }
    }

    if (! itemFound) {
        // We need a new item
        imageItem = new QListWidgetItem;
        imageItem->setIcon(QIcon(QPixmap::fromImage(m_imageCache->thumbnail(path))));
        imageItem->setData(Qt::UserRole, path);
    }

    QFileInfo info(path);
    imageItem->setText(m_imageCache->changed(path) ? i18n("*%1", info.fileName())
                                                   : info.fileName());

    KColorScheme scheme;
    switch (m_imageCache->matchType(path)) {
    case KGeoTag::MatchType::None:
        imageItem->setForeground(scheme.foreground());
        break;
    case KGeoTag::MatchType::Exact:
        imageItem->setForeground(scheme.foreground(KColorScheme::PositiveText));
        break;
    case KGeoTag::MatchType::Interpolated:
        imageItem->setForeground(scheme.foreground(KColorScheme::NeutralText));
        break;
    case KGeoTag::MatchType::Set:
        imageItem->setForeground(scheme.foreground(KColorScheme::LinkText));
        break;
    }

    if (! itemFound) {
        addItem(imageItem);
    }

    if (currentItem() == imageItem) {
        // Update the preview without centering the image
        emit imageSelected(path, false);
    }
}

QVector<QString> ImagesList::allImages() const
{
    QVector<QString> paths;
    for (int i = 0; i < count(); i++) {
        paths.append(item(i)->data(Qt::UserRole).toString());
    }
    return paths;
}

void ImagesList::removeImage(const QString &path)
{
    for (int i = 0; i < count(); i++) {
        if (item(i)->data(Qt::UserRole).toString() == path) {
            const auto *item = takeItem(i);
            delete item;
            return;
        }
    }
}

void ImagesList::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }

    QListWidget::mousePressEvent(event);
}

void ImagesList::mouseMoveEvent(QMouseEvent *event)
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

void ImagesList::showContextMenu(const QPoint &point)
{
    const auto *item = currentItem();
    if (item == nullptr) {
        return;
    }

    const QString path = item->data(Qt::UserRole).toString();
    m_removeCoordinates->setEnabled(m_imageCache->coordinates(path).isSet);
    m_discardChanges->setEnabled(m_imageCache->changed(path));

    m_contextMenu->exec(mapToGlobal(point));
}
