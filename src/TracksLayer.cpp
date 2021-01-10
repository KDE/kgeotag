/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "TracksLayer.h"

// Marble includes
#include <marble/GeoPainter.h>

static QStringList s_renderPosition { QStringLiteral("SURFACE") };

TracksLayer::TracksLayer(QObject *parent, QVector<Marble::GeoDataLineString> *tracks,
                         QPen *trackPen)
    : QObject(parent),
      m_tracks(tracks),
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

    for (const auto &lineString : *m_tracks) {
        painter->drawPolyline(lineString);
    }

    return true;
}
