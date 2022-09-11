// SPDX-FileCopyrightText: 2020-2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "MimeHelper.h"
#include "Logging.h"

// Qt includes
#include <QHash>
#include <QMimeDatabase>
#include <QMimeData>
#include <QUrl>

namespace MimeHelper
{

static const QVector<QString> s_usableImages {
    QStringLiteral("image/jpeg"),
    QStringLiteral("image/png"),
    QStringLiteral("image/webp"),
    QStringLiteral("image/tiff"),
    QStringLiteral("image/openraster"),
    QStringLiteral("application/x-krita")
};

static const QVector<QString> s_usableGeoData {
    QStringLiteral("application/x-gpx+xml"),
    QStringLiteral("application/xml+gpx")
};

static const QHash<KGeoTag::DropTarget, QVector<QString>> s_usableTypes {
    { KGeoTag::DroppedOnImageList, s_usableImages },
    { KGeoTag::DroppedOnMap,       s_usableGeoData },
    { KGeoTag::DroppedOnTrackList, s_usableGeoData }
};

static const QMimeDatabase s_mimeDB;

QVector<QString> getUsablePaths(KGeoTag::DropTarget dropTarget, const QMimeData *data)
{
    if (! data->hasUrls()) {
        return { };
    }

    QVector<QString> usablePaths;
    const auto urls = data->urls();
    for (const auto &url : urls) {
        if (! url.isLocalFile()) {
            continue;
        }

        const auto path = url.toLocalFile();
        const auto type = s_mimeDB.mimeTypeForFile(path);
        for (const auto &possibleType : s_usableTypes.value(dropTarget)) {
            if (type.inherits(possibleType)) {
                usablePaths.append(path);
                break;
            }
        }
    }

    return usablePaths;
}

QString mimeType(const QString &path)
{
    return s_mimeDB.mimeTypeForFile(path).name();
}

KGeoTag::FileType classifyFile(const QString &path)
{
    const auto type = s_mimeDB.mimeTypeForFile(path);
    logDebug << "MIME type for" << path << "is" << type.name();

    for (const auto &possibleType : s_usableImages) {
        if (type.inherits(possibleType)) {
            logDebug << type.name() << "inherits or is" << possibleType << "--> image file";
            return KGeoTag::ImageFile;
        }
    }

    for (const auto &possibleType : s_usableGeoData) {
        if (type.inherits(possibleType)) {
            logDebug << type.name() << "inherits or is" << possibleType << "--> geodata file";
            return KGeoTag::GeoDataFile;
        }
    }

    logDebug << type.name() << "can't be used";
    return KGeoTag::UnsupportedFile;
}

}
