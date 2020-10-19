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

#ifndef SETTINGS_H
#define SETTINGS_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QSettings>
#include <QColor>

class Settings : public QSettings
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent);

    void saveMainWindowGeometry(const QByteArray &data);
    QByteArray mainWindowGeometry() const;

    void saveMainWindowState(const QByteArray &data);
    QByteArray mainWindowState() const;

    void saveShowCrosshairs(bool state);
    bool showCrosshairs() const;

    void saveFloatersVisibility(const QHash<QString, bool> &data);
    QHash<QString, bool> floatersVisibility();

    void saveMapCenter(const KGeoTag::Coordinates &coordinates);
    KGeoTag::Coordinates mapCenter() const;

    void saveZoom(int zoom);
    int zoom() const;

    void saveLastOpenPath(const QString &path);
    QString lastOpenPath() const;

    void saveThumbnailSize(int size);
    QSize thumbnailSize() const;

    void savePreviewSize(int size);
    QSize previewSize() const;

    void saveExactMatchTolerance(int seconds);
    int exactMatchTolerance() const;

    void saveMaximumInterpolationInterval(int seconds);
    int maximumInterpolationInterval() const;

    void saveMaximumInterpolationDistance(int meters);
    int maximumInterpolationDistance() const;

    void saveLookupElevation(bool state);
    bool lookupElevation() const;

    void saveTrackColor(const QColor &color);
    QColor trackColor() const;

    void saveTrackWidth(int width);
    int trackWidth() const;

    void saveTrackStyle(Qt::PenStyle style);
    Qt::PenStyle trackStyle() const;

    void saveCreateBackups(bool state);
    bool createBackups() const;

};

#endif // SETTINGS_H
