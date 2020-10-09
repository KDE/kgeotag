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

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

// Qt includes
#include <QWidget>

// Local classes
class ImageCache;
class ImagePreview;

// Qt classes
class QLabel;

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(ImageCache *imageCache, QWidget *parent = nullptr);

public slots:
    void setImage(const QString &path);

private: // Variables
    ImageCache *m_imageCache;
    ImagePreview *m_preview;
    QLabel *m_path;
    QLabel *m_date;
    QLabel *m_coordinates;

};

#endif // PREVIEWWIDGET_H
