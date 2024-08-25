// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
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

// The distance between the lines of latitude (north-south position) is always the same.
// The distance between the lines of longitude (east-west position) depends on the latitude.
// At 0° N/S (at the equator), the distance is the biggest, at 90° N/S, it's 0.
//
// If we assume an earth circumference of 40,030,219 m, 1° latitude or longitude at the equator
// describes a distance of about 111,195 m.
//
// So if we want a worst-case (equator) precision of < 1 m (which should be way enough for what we
// do here), we need:

// 6 decimal places for decial degrees (--> ca. 11 cm precision)
constexpr const int degreesPrecision = 6;

// 4 decimal places for decimal minutes (--> ca. 19 cm precision)
constexpr const int minutesPrecision = 4;

// 2 decimal places for decimal seconds (--> ca. 31 cm precision)
constexpr const int secondsPrecision = 2;

// 10 cm altitude precision should be sufficient as well
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
