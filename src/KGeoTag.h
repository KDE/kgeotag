// SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef KGEOTAG_H
#define KGEOTAG_H

// Local includes
#include "Coordinates.h"

// Qt includes
#include <QString>
#include <QMetaType>
#include <QHash>

namespace KGeoTag
{

// Depth of the Marianna Trench minus some meters
constexpr const double minimalAltitude = -12000.0;

// Height of Mount Everest plus some meters
constexpr const double maximalAltitude = 8900.0;

// Earth radius according to the GRS 80 ellipsoid (radius of a sphere of equal area)
constexpr const double earthRadius = 6371007.2;

// 5 decimal places of degrees result in a precision of at worst about 1 m.
// This should be by far enough for the present use-case.
constexpr const int degreesPrecision = 5;

// Same for 0,1 m altitude precision ;-)
constexpr const int altitudePrecision = 1;

enum SearchType {
    CombinedMatchSearch,
    ExactMatchSearch,
    InterpolatedMatchSearch
};

enum MatchType {
    NotMatched,
    ExactMatch,
    InterpolatedMatch,
    ManuallySet
};

enum ImagesListType {
    UnAssignedImages,
    AssignedImages,
    AllImages
};

const QString backupSuffix = QStringLiteral("orig");

const auto SourceImagesListMimeType = QStringLiteral("application/x-kgeotag-source_images_list");
const QHash<ImagesListType, QByteArray> SourceImagesList {
    { ImagesListType::UnAssignedImages, QByteArray("UnAssignedImages") },
    { ImagesListType::AssignedImages,   QByteArray("AssignedImages") },
    { ImagesListType::AllImages,        QByteArray("AllImages") }
};

enum CustomDataRoles {
    PathRole = Qt::UserRole,
    DateRole,
    CoordinatesRole,
    ThumbnailRole,
    PreviewRole,
    MatchTypeRole,
    ChangedRole
};

enum DropTarget {
    DroppedOnImageList,
    DroppedOnMap,
    DroppedOnTrackList
};

enum FileType {
    UnsupportedFile,
    ImageFile,
    GeoDataFile
};

}

Q_DECLARE_METATYPE(KGeoTag::MatchType)

#endif // KGEOTAG_H
