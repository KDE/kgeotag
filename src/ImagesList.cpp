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
#include "ImagesList.h"
#include "ImageCache.h"
#include "ImageItem.h"
#include "Settings.h"

// KDE includes
#include <KLocalizedString>

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
#include <QKeyEvent>

ImagesList::ImagesList(ImagesList::Type type, Settings *settings, ImageCache *imageCache,
                       QWidget *parent)
    : QListWidget(parent),
      m_type(type),
      m_settings(settings),
      m_imageCache(imageCache)
{
    setSortingEnabled(true);
    setIconSize(m_imageCache->thumbnailSize());

    connect(this, &QListWidget::currentItemChanged, this, &ImagesList::imageHighlighted);
    if (m_type == Type::Assigned) {
        connect(this, &QListWidget::itemClicked,
                [this](QListWidgetItem *item)
                {
                    emit centerImage(dynamic_cast<ImageItem *>(item)->path());
                });
    }

    m_contextMenu = new QMenu(this);

    auto *assignToMapCenterAction = m_contextMenu->addAction(i18n("Assign to map center"));
    connect(assignToMapCenterAction, &QAction::triggered,
            [this]
            {
                emit assignToMapCenter(dynamic_cast<ImageItem *>(currentItem())->path());
            });

    m_contextMenu->addSeparator();

    if (m_type == Type::Assigned) {
        m_lookupElevation = m_contextMenu->addAction(i18n("Lookup elevation"));
        connect(m_lookupElevation, &QAction::triggered,
                [this]
                {
                    emit lookupElevation(dynamic_cast<ImageItem *>(currentItem())->path());
                });
    }

    m_removeCoordinates = m_contextMenu->addAction(i18n("Remove coordinates"));
    connect(m_removeCoordinates, &QAction::triggered,
            [this]
            {
                emit removeCoordinates(dynamic_cast<ImageItem *>(currentItem())->path());
            });

    m_discardChanges = m_contextMenu->addAction(i18n("Discard changes"));
    connect(m_discardChanges, &QAction::triggered,
            [this]
            {
                emit discardChanges(dynamic_cast<ImageItem *>(currentItem())->path());
            });

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QListWidget::customContextMenuRequested, this, &ImagesList::showContextMenu);
}

void ImagesList::keyPressEvent(QKeyEvent *event)
{
    if (m_type == Type::Assigned) {
        const auto key = event->key();
        if (key == Qt::Key_Up || key == Qt::Key_Down
            || key == Qt::Key_PageUp || key == Qt::Key_PageDown) {

            m_itemBeforeKeyPress = currentItem();
        }
    }

    QListWidget::keyPressEvent(event);
}

void ImagesList::keyReleaseEvent(QKeyEvent *event)
{
    if (m_type == Type::Assigned) {
        const auto *item = currentItem();
        if (m_itemBeforeKeyPress != nullptr && m_itemBeforeKeyPress != item) {
            emit centerImage(dynamic_cast<const ImageItem *>(item)->path());
        }
        m_itemBeforeKeyPress = nullptr;
    }

    QListWidget::keyReleaseEvent(event);
}

void ImagesList::imageHighlighted(QListWidgetItem *item, QListWidgetItem *) const
{
    if (item == nullptr) {
        return;
    }

    emit imageSelected(dynamic_cast<ImageItem *>(item)->path());
}

void ImagesList::addOrUpdateImage(const QString &path)
{
    bool itemFound = false;
    ImageItem *imageItem;

    const QFileInfo info(path);
    const QString fileName = info.fileName();

    // Check for an existing entry we want to update
    for (int i = 0; i < count(); i++) {
        if (dynamic_cast<ImageItem *>(item(i))->path() == path) {
            imageItem = dynamic_cast<ImageItem *>(item(i));
            itemFound = true;
            break;
        }
    }

    if (! itemFound) {
        // We need a new item
        imageItem = new ImageItem(QIcon(QPixmap::fromImage(m_imageCache->thumbnail(path))),
                                  fileName, path);
    }

    imageItem->setChanged(m_imageCache->changed(path));
    imageItem->setMatchType(m_imageCache->matchType(path));

    if (! itemFound) {
        addItem(imageItem);
    }

    if (currentItem() == imageItem) {
        emit imageSelected(path);
    }
}

QVector<QString> ImagesList::allImages() const
{
    QVector<QString> paths;
    for (int i = 0; i < count(); i++) {
        paths.append(dynamic_cast<ImageItem *>(item(i))->path());
    }
    return paths;
}

void ImagesList::removeImage(const QString &path)
{
    for (int i = 0; i < count(); i++) {
        if (dynamic_cast<ImageItem *>(item(i))->path() == path) {
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

    const auto path = dynamic_cast<const ImageItem *>(item)->path();
    emit imageSelected(path);

    auto *drag = new QDrag(this);
    drag->setPixmap(item->icon().pixmap(iconSize()));

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(path);
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}

void ImagesList::showContextMenu(const QPoint &point)
{
    const auto *item = currentItem();
    if (item == nullptr) {
        return;
    }

    if (m_lookupElevation != nullptr) {
        m_lookupElevation->setEnabled(m_settings->lookupElevation());
    }

    const QString &path = dynamic_cast<const ImageItem *>(item)->path();
    m_removeCoordinates->setEnabled(m_imageCache->coordinates(path).isSet);
    m_discardChanges->setEnabled(m_imageCache->changed(path));

    m_contextMenu->exec(mapToGlobal(point));
}
