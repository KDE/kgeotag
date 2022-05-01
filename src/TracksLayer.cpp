// SPDX-FileCopyrightText: 2021 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

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

    for (const auto &segments : m_geoDataModel->marbleTracks()) {
        for (const auto &segment : segments) {
            painter->drawPolyline(segment);
        }
    }

    return true;
}
