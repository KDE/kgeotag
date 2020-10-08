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

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QListWidget>

ImagesWidget::ImagesWidget(Settings *settings, QWidget *parent)
    : QWidget(parent), m_settings(settings)
{
    auto *layout = new QVBoxLayout(this);

    m_images = new QListWidget;
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

    for (const auto &file : files) {
        if (m_allImages.contains(file)) {
            continue;
        }

        const QFileInfo info(file);
        m_allImages.append(file);

        auto *item = new QListWidgetItem(info.fileName());
        item->setData(Qt::UserRole, m_allImages.count() - 1);
        m_images->addItem(item);
    }
}
