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

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

// Qt includes
#include <QLabel>
#include <QImage>

// Local classes
class ImageCache;

// Qt classes
class QTimer;

class ImagePreview : public QLabel
{
    Q_OBJECT

public:
    explicit ImagePreview(ImageCache *imageCache, QWidget *parent = nullptr);
    void setImage(const QString &path);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void setScaledPreview();

private: // Variables
    ImageCache *m_imageCache;
    QImage m_currentImage;
    QTimer *m_smoothTimer;

};

#endif // IMAGEPREVIEW_H
