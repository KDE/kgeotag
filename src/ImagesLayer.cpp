/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "ImagesLayer.h"
#include "ImagesModel.h"

// Marble includes
#include <marble/GeoPainter.h>
#include <marble/ViewportParams.h>
#include <marble/GeoDataLatLonAltBox.h>

// Qt includes
#include <QDebug>

static QStringList s_renderPosition { QStringLiteral("HOVERS_ABOVE_SURFACE") };

ImagesLayer::ImagesLayer(QObject *parent, ImagesModel *model)
    : QObject(parent),
      m_imagesModel(model)
{
}

QStringList ImagesLayer::renderPosition() const
{
    return s_renderPosition;
}

bool ImagesLayer::render(Marble::GeoPainter *painter, Marble::ViewportParams *viewport,
                         const QString &, Marble::GeoSceneLayer *)
{
    const auto viewportCoordinates = viewport->viewLatLonAltBox();

    for (int row = 0; row < m_imagesModel->rowCount(); row++) {
        const auto index = m_imagesModel->index(row, 0);
        const auto coordinates = index.data(KGeoTag::CoordinatesRole).value<Coordinates>();
        if (! coordinates.isSet()) {
            continue;
        }

        const auto marbleCoordinates = Marble::GeoDataCoordinates(
            coordinates.lon(), coordinates.lat(), coordinates.alt(),
            Marble::GeoDataCoordinates::Degree);

        if (! viewportCoordinates.contains(marbleCoordinates)) {
            continue;
        }

        painter->drawPixmap(marbleCoordinates, index.data(KGeoTag::ThumbnailRole).value<QPixmap>());
    }

    return true;
}
