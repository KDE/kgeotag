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

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QListWidget>

// Local classes
class ImageCache;

// Qt classes
class QMouseEvent;
class QMenu;
class QAction;

class ImagesList : public QListWidget
{
    Q_OBJECT

public:
    explicit ImagesList(ImageCache *imageCache, QWidget *parent = nullptr);
    void addOrUpdateImage(const QString &path);
    QVector<QString> allImages() const;
    void removeImage(const QString &path);

signals:
    void imageSelected(const QString &path, bool center = true) const;
    void removeCoordinates(const QString &path) const;
    void discardChanges(const QString &path) const;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void imageHighlighted(QListWidgetItem *item, QListWidgetItem *) const;
    void showContextMenu(const QPoint &point);

private: // Variables
    ImageCache *m_imageCache;
    QPoint m_dragStartPosition;
    QMenu *m_contextMenu;
    QAction *m_removeCoordinates;
    QAction *m_discardChanges;

};

#endif // IMAGESLIST_H
