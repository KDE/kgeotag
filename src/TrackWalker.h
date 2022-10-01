// SPDX-FileCopyrightText: 2021-2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef TRACKWALKER_H
#define TRACKWALKER_H

// Qt includes
#include <QWidget>

// Local classes
class GeoDataModel;

// Qt classes
class QSlider;
class QLabel;

class TrackWalker : public QWidget
{
    Q_OBJECT

public:
    explicit TrackWalker(GeoDataModel *model, QWidget *parent = nullptr);

public Q_SLOTS:
    void setToTrack(int row);

Q_SIGNALS:
    void trackPointSelected(int trackIndex, int trackPointIndex);

private Q_SLOTS:
    void sliderMoved(int index);

private: // Variables
    GeoDataModel *m_geoDataModel;
    QSlider *m_slider;
    QLabel *m_info;
    int m_trackIndex = -1;

};

#endif // TRACKWALKER_H
