//* SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef SETTINGS_H
#define SETTINGS_H

// Local includes
#include "KGeoTag.h"
#include "Coordinates.h"

// KDE includes
#include <KSharedConfig>

// Qt includes
#include <QColor>
#include <QHash>

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent);

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

    void saveLookupElevationAutomatically(bool state);
    bool lookupElevationAutomatically() const;

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

    void saveAllowWriteRawFiles(bool state);
    bool allowWriteRawFiles() const;

    void saveCreateBackups(bool state);
    bool createBackups() const;

    void saveBookmarks(const QHash<QString, Coordinates> *bookmarks);
    QHash<QString, Coordinates> bookmarks() const;

    void saveDefaultMatchingMode(KGeoTag::SearchType mode);
    KGeoTag::SearchType defaultMatchingMode() const;

    void saveSelectNextUntagged(bool state);
    bool selectNextUntagged() const;

private: // Variables
    KSharedConfig::Ptr m_config;

};

#endif // SETTINGS_H
