/* SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

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
    QVector<QString> changedImages() const;
    QDateTime date(const QString &path) const;
    KGeoTag::MatchType matchType(const QString &path) const;
    void setCoordinates(const QString &path, const Coordinates &coordinates,
                        KGeoTag::MatchType matchType);
    void setElevation(const QString &path, double elevation);
    Coordinates coordinates(const QString &path) const;
    void resetChanges(const QString &path);
    void setSaved(const QString &path);

private: // Functions
    void emitDataChanged(const QString &path);

private: // Variables
    struct ImageData {
        QString fileName;
        QDateTime date;
        Coordinates originalCoordinates;
        Coordinates coordinates;
        QPixmap thumbnail;
        QImage preview;
        KGeoTag::MatchType matchType = KGeoTag::NotMatched;
    };

    bool m_splitImagesList;
    QSize m_thumbnailSize;
    QSize m_previewSize;

    KColorScheme m_colorScheme;
    QVector<QString> m_paths;
    QHash<QString, ImageData> m_imageData;

};

#endif // IMAGESMODEL_H
