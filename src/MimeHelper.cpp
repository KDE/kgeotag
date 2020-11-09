/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>
    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "MimeHelper.h"

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

static const QHash<KGeoTag::DropTarget, QVector<QString>> s_usableTypes {
    { KGeoTag::DroppedOnImageList, s_usableImages },
    { KGeoTag::DroppedOnMap,       { QStringLiteral("application/x-gpx+xml") } }
};

static const QMimeDatabase s_mimeDB;

QVector<QString> getUsablePaths(KGeoTag::DropTarget dropTarget, const QMimeData *data)
{
    if (! data->hasUrls()) {
        return { };
    }

    QVector<QString> usablePaths;
    const auto urls = data->urls();
    for (const auto url : urls) {
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

bool mimeTypeOkay(const QString &path)
{
    const auto type = s_mimeDB.mimeTypeForFile(path);
    for (const auto &possibleType : s_usableImages) {
        if (type.inherits(possibleType)) {
            return true;
        }
    }
    return false;
}

QString mimeType(const QString &path)
{
    return s_mimeDB.mimeTypeForFile(path).name();
}

}
