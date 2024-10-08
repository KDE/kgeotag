// SPDX-FileCopyrightText: 2021-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "TrackWalker.h"
#include "GeoDataModel.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>

TrackWalker::TrackWalker(GeoDataModel *model, QWidget *parent)
    : QWidget(parent),
      m_geoDataModel(model)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    auto *layout = new QVBoxLayout(this);

    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setMinimum(1);
    connect(m_slider, &QAbstractSlider::valueChanged, this, &TrackWalker::sliderMoved);
    layout->addWidget(m_slider);

    m_info = new QLabel(i18n("<i>No track selected</i>"));
    layout->addWidget(m_info);

    setEnabled(false);
}

void TrackWalker::setToTrack(int row)
{
    m_trackIndex = row;

    const int count = row != -1 ? m_geoDataModel->dateTimes().at(row).count() : 1;
    m_slider->blockSignals(true);
    m_slider->setValue(1);
    m_slider->setMaximum(count);
    m_slider->blockSignals(false);

    m_info->setText(row != -1 ? i18np("%1 trackpoint", "%1 trackpoints", count)
                              : i18n("<i>No track selected</i>"));

    setEnabled(row != -1);
}

void TrackWalker::sliderMoved(int index)
{
    m_info->setText(i18n("Selected trackpoint %1 of %2", index, m_slider->maximum()));
    Q_EMIT trackPointSelected(m_trackIndex, index - 1);
}
