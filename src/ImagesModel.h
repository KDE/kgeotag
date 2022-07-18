// SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef IMAGESMODEL_H
#define IMAGESMODEL_H

// Local includes
#include "KGeoTag.h"

// KDE includes
#include <KColorScheme>

// Qt includes
#include <QAbstractListModel>
#include <QDateTime>
#include <QImage>
#include <QSize>
#include <QTimeZone>

class ImagesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum LoadResult {
        LoadingImageFailed,
        LoadingMetadataFailed,
        AlreadyLoaded,
        LoadingSucceeded
    };

    explicit ImagesModel(QObject *parent, bool splitImagesList, int thumbnailSize, int previewSize);

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setSplitImagesList(bool state);
    QModelIndex indexFor(const QString &path) const;
    bool contains(const QString &path) const;
    LoadResult addImage(const QString &path);
    const QVector<QString> &allImages() const;
    QVector<QString> imagesWithPendingChanges() const;
    QVector<QString> processedSavedImages() const;
    QVector<QString> imagesLoadedTagged() const;
    QDateTime date(const QString &path) const;
    KGeoTag::MatchType matchType(const QString &path) const;
    void setCoordinates(const QString &path, const Coordinates &coordinates,
                        KGeoTag::MatchType matchType);
    void setElevation(const QString &path, double elevation);
    Coordinates coordinates(const QString &path) const;
    void resetChanges(const QString &path);
    void setSaved(const QString &path);
    void setImagesTimeZone(const QByteArray &id);
    bool hasPendingChanges(const QString &path) const;
    void removeImages(const QVector<QString> &paths);
    void removeAllImages();

private: // Functions
    void emitDataChanged(const QString &path);

private: // Variables
    struct ImageData {
        QString fileName;
        QDateTime date;
        Coordinates originalCoordinates;
        Coordinates lastSavedCoordinates;
        Coordinates coordinates;
        QPixmap thumbnail;
        QImage preview;
        KGeoTag::MatchType matchType = KGeoTag::NotMatched;
        bool changed = false;
    };

    bool m_splitImagesList;
    QSize m_thumbnailSize;
    QSize m_previewSize;

    KColorScheme m_colorScheme;
    QVector<QString> m_paths;
    QHash<QString, ImageData> m_imageData;
    QTimeZone m_timeZone;

};

#endif // IMAGESMODEL_H
