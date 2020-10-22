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

#ifndef IMAGESLIST_H
#define IMAGESLIST_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QListWidget>

// Local classes
class SharedObjects;
class Settings;
class ImageCache;
class ElevationEngine;

// Qt classes
class QMouseEvent;
class QMenu;
class QAction;
class QKeyEvent;

class ImagesList : public QListWidget
{
    Q_OBJECT

public:
    enum Type {
        UnAssigned,
        Assigned
    };

    explicit ImagesList(SharedObjects *sharedObjects, Type type, QWidget *parent = nullptr);
    void addOrUpdateImage(const QString &path);
    QVector<QString> allImages() const;
    void removeImage(const QString &path);

public slots:
    void lookupElevation(const QString &path = QString());

signals:
    void imageSelected(const QString &path) const;
    void centerImage(const QString &path) const;
    void assignToMapCenter(const QString &path) const;
    void removeCoordinates(const QString &path) const;
    void discardChanges(const QString &path) const;
    void checkUpdatePreview(const QString &path) const;

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void imageHighlighted(QListWidgetItem *item, QListWidgetItem *) const;
    void showContextMenu(const QPoint &point);
    void elevationProcessed(QString path, bool success, double elevation);

private: // Variables
    Settings *m_settings;
    ImageCache *m_imageCache;
    ElevationEngine *m_elevationEngine;
    Type m_type;

    QListWidgetItem *m_itemBeforeKeyPress = nullptr;
    QPoint m_dragStartPosition;

    QMenu *m_contextMenu;
    QAction *m_lookupElevation = nullptr;
    QAction *m_removeCoordinates;
    QAction *m_discardChanges;

};

#endif // IMAGESLIST_H
