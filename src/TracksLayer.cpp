/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "TracksLayer.h"
#include "GeoDataModel.h"
#include "KGeoTag.h"

// Marble includes
#include <marble/GeoPainter.h>

// C++ includes
#include <utility>

static QStringList s_renderPosition { QStringLiteral("SURFACE") };

TracksLayer::TracksLayer(QObject *parent, GeoDataModel *geoDataModel, QPen *trackPen)
    : QObject(parent),
      m_geoDataModel(geoDataModel),
      m_trackPen(trackPen)
{
}

QStringList TracksLayer::renderPosition() const
{
    return s_renderPosition;
}

bool TracksLayer::render(Marble::GeoPainter *painter, Marble::ViewportParams *, const QString &,
                         Marble::GeoSceneLayer *)
{
    painter->setPen(*m_trackPen);

    for (int i = 0; i < m_geoDataModel->rowCount(); i++) {
        const auto data = m_geoDataModel->data(m_geoDataModel->index(i, 0),
                                               KGeoTag::DisplayTracksRole);
        const auto track = data.value<QVector<Marble::GeoDataLineString>>();
        for (const auto &segment : track) {
            painter->drawPolyline(segment);
        }
    }

    return true;
}
