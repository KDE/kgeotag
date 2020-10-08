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
#include "ImagesWidget.h"
#include "Settings.h"
#include "ImageCache.h"
#include "ImagesList.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>
#include <QApplication>
#include <QIcon>

ImagesWidget::ImagesWidget(Settings *settings, ImageCache *imageCache, QWidget *parent)
    : QWidget(parent), m_settings(settings), m_imageCache(imageCache)
{
    auto *layout = new QVBoxLayout(this);

    m_images = new ImagesList;
    m_images->setSortingEnabled(true);
    m_images->setIconSize(m_settings->thumbnailSize());
    connect(m_images, &QListWidget::itemClicked, [this](QListWidgetItem *item)
            {
                emit imageSelected(item->data(Qt::UserRole).toString());
            });
    layout->addWidget(m_images);
}

void ImagesWidget::addImages()
{
    const auto files = QFileDialog::getOpenFileNames(this,
                           i18n("Please select the images to add"),
                           m_settings->lastImagesOpenPath(),
                           i18n("JPEG Images (*.jpg *.jpeg)"));
    if (files.isEmpty()) {
        return;
    }

    const QFileInfo info(files.at(0));
    m_settings->saveLastImagesOpenPath(info.dir().absolutePath());

    QApplication::setOverrideCursor(Qt::WaitCursor);

    for (const auto &path : files) {
        const QFileInfo info(path);
        const QString id = info.canonicalFilePath();

        if (! m_imageCache->addImage(id)) {
            continue;
        }

        auto *item = new QListWidgetItem(QIcon(QPixmap::fromImage(m_imageCache->thumbnail(id))),
                                         info.fileName());
        item->setData(Qt::UserRole, id);
        m_images->addItem(item);
    }

    QApplication::restoreOverrideCursor();
}
