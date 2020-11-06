/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SETTINGS_H
#define SETTINGS_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// Qt includes
#include <QSettings>
#include <QColor>
#include <QHash>

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

    void saveMapCenter(const Coordinates &coordinates);
    Coordinates mapCenter() const;

    void saveZoom(int zoom);
    int zoom() const;

    void saveLastOpenPath(const QString &path);
    QString lastOpenPath() const;

    void saveSplitImagesList(bool state);
    bool splitImagesList() const;

    void saveThumbnailSize(int size);
    int thumbnailSize() const;

    void savePreviewSize(int size);
    int previewSize() const;

    void saveExactMatchTolerance(int seconds);
    int exactMatchTolerance() const;

    void saveMaximumInterpolationInterval(int seconds);
    int maximumInterpolationInterval() const;

    void saveMaximumInterpolationDistance(int meters);
    int maximumInterpolationDistance() const;

    void saveExcludeManuallyTaggedWhenReassigning(bool state);
    bool excludeManuallyTaggedWhenReassigning() const;

    void saveLookupElevation(bool state);
    bool lookupElevation() const;

    void saveElevationDataset(const QString &id);
    QString elevationDataset() const;

    void saveTrackColor(const QColor &color);
    QColor trackColor() const;

    void saveTrackWidth(int width);
    int trackWidth() const;

    void saveTrackStyle(Qt::PenStyle style);
    Qt::PenStyle trackStyle() const;

    void saveWriteMode(const QString &writeMode);
    QString writeMode() const;

    void saveCreateBackups(bool state);
    bool createBackups() const;

    void saveBookmarks(const QHash<QString, Coordinates> *bookmarks);
    QHash<QString, Coordinates> bookmarks() const;

};

#endif // SETTINGS_H
