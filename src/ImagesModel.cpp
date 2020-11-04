/* Copyright (C) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// Local includes
#include "ImagesModel.h"
#include "SharedObjects.h"
#include "Settings.h"
#include "KGeoTag.h"

// KDE includes
#include <KLocalizedString>
#include <KExiv2/KExiv2>

// Qt includes
#include <QFileInfo>
#include <QFont>

// C++ includes
#include <utility>

ImagesModel::ImagesModel(QObject *parent, SharedObjects *sharedObjects)
    : QAbstractListModel(parent),
      m_settings(sharedObjects->settings())
{
}

int ImagesModel::rowCount(const QModelIndex &) const
{
    return m_paths.count();
}

QVariant ImagesModel::data(const QModelIndex &index, int role) const
{
    if (! index.isValid() || index.row() > m_paths.count()) {
        return QVariant();
    }

    const auto &path = m_paths.at(index.row());
    const auto &data = m_imageData[path];

    if (role == Qt::DisplayRole) {
        const QString associatedMarker = (! m_settings->splitImagesList()
                                          && data.coordinates != KGeoTag::NoCoordinates)
            ? i18nc("Marker for an associated file", "\u2713\u2009")
            : QString();
        const QString changedmarker = data.coordinates != data.originalCoordinates
            ? i18nc("Marker for a changed file", "\u2009*")
            : QString();
        return i18nc("Pattern for a display filename with a \"changed\" and a \"associated\" "
                     "marker. The first option is the \"associated\" marker, the second one the "
                     "filename and the third one the \"changed\" marker.",
                     "%1%2%3",
                     associatedMarker, data.fileName, changedmarker);

    } else if (role == Qt::DecorationRole) {
        return data.thumbnail;

    } else if (role == Qt::ForegroundRole) {
        switch (data.matchType) {
        case KGeoTag::NotMatched:
            return m_colorScheme.foreground();
        case KGeoTag::ExactMatch:
            return m_colorScheme.foreground(KColorScheme::PositiveText);
        case KGeoTag::InterpolatedMatch:
            return m_colorScheme.foreground(KColorScheme::NeutralText);
        case KGeoTag::ManuallySet:
            return m_colorScheme.foreground(KColorScheme::LinkText);
        }

    } else if (! m_settings->splitImagesList() && role == Qt::FontRole) {
        QFont font;
        if (data.coordinates != KGeoTag::NoCoordinates) {
            font.setBold(true);
        }
        return font;

    } else if (role == DataRole::Path) {
        return path;

    } else if (role == DataRole::Date) {
        return data.date;

    } else if (role == DataRole::Coordinates) {
        QVariant coordinates;
        coordinates.setValue(data.coordinates);
        return coordinates;

    } else if (role == DataRole::Thumbnail) {
        return data.thumbnail;

    } else if (role == DataRole::Preview) {
        return data.preview;

    } else if (role == DataRole::MatchType) {
        QVariant matchType;
        matchType.setValue(data.matchType);
        return matchType;

    } else if (role == DataRole::Changed) {
        return data.originalCoordinates != data.coordinates;

    }

    return QVariant();
}

bool ImagesModel::addImage(const QString &path)
{
    // Check if we already have the image
    if (m_paths.contains(path)) {
        return true;
    }

    // Read the image
    QImage image = QImage(path);
    if (image.isNull()) {
        return false;
    }

    // Read the exif data
    auto exif = KExiv2Iface::KExiv2();
    exif.setUseXMPSidecar4Reading(true);
    if (! exif.load(path)) {
        return false;
    }

    // Prepare the images's data struct
    ImageData data;

    // Add the filename
    const QFileInfo info(path);
    data.fileName = info.fileName();

    // Read the date (falling back to the file's date if nothing is set)
    data.date = exif.getImageDateTime();

    // Try to read gps information
    double altitude;
    double latitude;
    double longitude;
    if (exif.getGPSInfo(altitude, latitude, longitude)) {
        data.originalCoordinates = KGeoTag::Coordinates { longitude, latitude, altitude, true };
        data.coordinates = KGeoTag::Coordinates { longitude, latitude, altitude, true };
    }

    // Fix the image's orientation
    exif.rotateExifQImage(image, exif.getImageOrientation());

    // Create a smaller thumbnail
    data.thumbnail = image.scaled(m_settings->thumbnailSize(), Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);

    // Create a bigger preview (to be scaled according to the view size)
    data.preview = image.scaled(m_settings->previewSize(), Qt::KeepAspectRatio);

    // Find the correct row for the new image (sorted by date)
    int row = 0;
    for (const QString &path : m_paths) {
        if (m_imageData.value(path).date > data.date) {
            break;
        }
        row++;
    }

    // Add the image

    beginInsertRows(QModelIndex(), row, row);
    m_paths.insert(row, path);
    m_imageData.insert(path, data);
    endInsertRows();

    const auto modelIndex = index(row, 0, QModelIndex());
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });

    return true;
}

void ImagesModel::emitDataChanged(const QString &path)
{
    const auto modelIndex = indexFor(path);
    emit dataChanged(modelIndex, modelIndex, { Qt::DisplayRole });
}

void ImagesModel::setMatchType(const QString &path, int matchType)
{
    m_imageData[path].matchType = matchType;
    emitDataChanged(path);
}

QVector<QString> ImagesModel::changedImages() const
{
    QVector<QString> changed;
    for (const auto &path : std::as_const(m_paths)) {
        const auto &data = m_imageData[path];
        if (data.coordinates != data.originalCoordinates) {
            changed.append(path);
        }
    }
    return changed;
}

QImage ImagesModel::thumbnail(const QString &path) const
{
    return m_imageData.value(path).thumbnail;
}

QDateTime ImagesModel::date(const QString &path) const
{
    return m_imageData.value(path).date;
}

bool ImagesModel::contains(const QString &path) const
{
    return m_paths.contains(path);
}

KGeoTag::Coordinates ImagesModel::coordinates(const QString &path) const
{
    return m_imageData.value(path).coordinates;
}

void ImagesModel::setCoordinates(const QString &path, double lon, double lat, double alt)
{
    setCoordinates(path, KGeoTag::Coordinates { lon, lat, alt, true });
}

void ImagesModel::setCoordinates(const QString &path, const KGeoTag::Coordinates &coordinates)
{
    m_imageData[path].coordinates = coordinates;
}

void ImagesModel::resetChanges(const QString &path)
{
    auto &data = m_imageData[path];
    data.coordinates = data.originalCoordinates;
    data.matchType = KGeoTag::NotMatched;
    emitDataChanged(path);
}

QModelIndex ImagesModel::indexFor(const QString &path) const
{
    return index(m_paths.indexOf(path), 0, QModelIndex());
}

void ImagesModel::setSaved(const QString &path)
{
    auto &data = m_imageData[path];
    data.originalCoordinates = data.coordinates;
}
