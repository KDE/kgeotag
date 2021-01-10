/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TRACKSLAYER_H
#define TRACKSLAYER_H

// Marble includes
#include <marble/LayerInterface.h>
#include <marble/GeoDataLineString.h>

// Qt includes
#include <QObject>

// Marble classes
namespace Marble
{
class GeoPainter;
class ViewportParams;
class GeoSceneLayer;
}

// Qt classes
class QPen;

class TracksLayer : public QObject, public Marble::LayerInterface
{
    Q_OBJECT

public:
    TracksLayer(QObject *parent, QVector<Marble::GeoDataLineString> *tracks, QPen *trackPen);
    QStringList renderPosition() const override;
    bool render(Marble::GeoPainter *painter, Marble::ViewportParams *,
                const QString &, Marble::GeoSceneLayer *) override;

private: // Variables
    const QVector<Marble::GeoDataLineString> *m_tracks;
    const QPen *m_trackPen;

};

#endif // TRACKSLAYER_H
