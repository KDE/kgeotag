// SPDX-FileCopyrightText: 2021 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#ifndef IMAGESLAYER_H
#define IMAGESLAYER_H

// Marble includes
#include <marble/LayerInterface.h>

// Qt includes
#include <QObject>

// Local classes
class ImagesModel;

// Marble classes
namespace Marble
{
class GeoPainter;
class ViewportParams;
class GeoSceneLayer;
}

class ImagesLayer : public QObject, public Marble::LayerInterface
{
    Q_OBJECT

public:
    ImagesLayer(QObject *parent, ImagesModel *model);
    QStringList renderPosition() const override;
    bool render(Marble::GeoPainter *painter, Marble::ViewportParams *viewport,
                const QString &, Marble::GeoSceneLayer *) override;

private: // Variables
    ImagesModel *m_imagesModel;

};

#endif // IMAGESLAYER_H
