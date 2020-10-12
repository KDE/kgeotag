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

    void saveFloatersVisibility(const QHash<QString, bool> &data);
    QHash<QString, bool> floatersVisibility();

    void saveMapCenter(const KGeoTag::Coordinates &coordinates);
    KGeoTag::Coordinates mapCenter() const;

    void saveZoom(int zoom);
    int zoom() const;

    void saveLastOpenPath(const QString &path);
    QString lastOpenPath() const;

    QSize thumbnailSize() const;
    QSize previewSize() const;

    void saveExactMatchDeviation(int seconds);
    int exactMatchDeviation() const;

    void saveMaximumInterpolationInterval(int seconds);
    int maximumInterpolationInterval() const;

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
