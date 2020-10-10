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

#ifndef IMAGESLIST_H
#define IMAGESLIST_H

// Qt includes
#include <QListWidget>

// Local classes
class ImageCache;

// Qt classes
class QMouseEvent;

class ImagesList : public QListWidget
{
    Q_OBJECT

public:
    explicit ImagesList(ImageCache *imageCache, QWidget *parent = nullptr);
    void addImage(const QString fileName, const QString &path);
    QVector<QString> allImages() const;
    void removeImage(const QString &path);

signals:
    void imageSelected(const QString &path) const;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private: // Variables
    ImageCache *m_imageCache;
    QPoint m_dragStartPosition;

};

#endif // IMAGESLIST_H
