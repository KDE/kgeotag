/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "DropHelper.h"

// Qt includes
#include <QHash>
#include <QMimeDatabase>
#include <QMimeData>
#include <QUrl>

namespace DropHelper
{

static const QHash<KGeoTag::DropTarget, QVector<QString>> s_usableTypes {
    { KGeoTag::DroppedOnImageList, {
        QStringLiteral("image/jpeg"),
        QStringLiteral("image/png"),
        QStringLiteral("image/webp"),
        QStringLiteral("image/tiff"),
        QStringLiteral("image/openraster"),
        QStringLiteral("application/x-krita")
    } },

    { KGeoTag::DroppedOnMap, {
        QStringLiteral("application/x-gpx+xml")
    } }
};

QVector<QString> getUsablePaths(KGeoTag::DropTarget dropTarget, const QMimeData *data)
{
    if (! data->hasUrls()) {
        return { };
    }

    const QMimeDatabase mimeDB;

    QVector<QString> usbalePaths;
    const auto urls = data->urls();
    for (const auto url : urls) {
        if (! url.isLocalFile()) {
            continue;
        }

        const auto path = url.toLocalFile();
        const auto type = mimeDB.mimeTypeForFile(path);
        for (const auto &possibleType : s_usableTypes.value(dropTarget)) {
            if (type.inherits(possibleType)) {
                usbalePaths.append(path);
                break;
            }
        }
    }

    return usbalePaths;
}

}
