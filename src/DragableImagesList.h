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

#ifndef DRAGABLEIMAGESLIST_H
#define DRAGABLEIMAGESLIST_H

// Local includes
#include "ImagesList.h"

// Local classes
class ImageCache;

// Qt classes
class QMouseEvent;

class DragableImagesList : public ImagesList
{
    Q_OBJECT

public:
    explicit DragableImagesList(ImageCache *imageCache, QWidget *parent = nullptr);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private: // Variables
    QPoint m_dragStartPosition;

};

#endif // DRAGABLEIMAGESLIST_H
