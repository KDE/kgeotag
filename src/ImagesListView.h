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

#ifndef IMAGESLISTVIEW_H
#define IMAGESLISTVIEW_H

// Qt includes
#include <QListView>

// Local classes
class SharedObjects;
class ImageCache;

// Qt classes
class QMenu;

class ImagesListView : public QListView
{
    Q_OBJECT

public:
    explicit ImagesListView(SharedObjects *sharedObjects, QWidget *parent = nullptr);
    QVector<QString> selectedPaths() const;

signals:
    void imageSelected(const QString &path) const;
    void centerImage(const QString &path) const;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

private slots:
    void checkCenterImage(const QString &path) const;

private: // Variables
    ImageCache *m_imageCache;

    bool m_dragStarted = false;
    QPoint m_dragStartPosition;

};

#endif // IMAGESLISTVIEW_H
