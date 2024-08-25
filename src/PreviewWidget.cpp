// SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "PreviewWidget.h"
#include "SharedObjects.h"
#include "CoordinatesFormatter.h"
#include "ImagePreview.h"
#include "Coordinates.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QLocale>
#include <QGridLayout>
#include <QDateTime>
#include <QToolButton>
#include <QDesktopServices>
#include <QUrl>

PreviewWidget::PreviewWidget(SharedObjects *sharedObjects, QWidget *parent)
    : QWidget(parent),
      m_formatter(sharedObjects->coordinatesFormatter()),
      m_locale(sharedObjects->locale())
{
    auto *layout = new QVBoxLayout(this);

    auto *infoLayout = new QGridLayout;
    layout->addLayout(infoLayout);

    auto *pathLabel = new QLabel(i18n("Image:"));
    pathLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    infoLayout->addWidget(pathLabel, 0, 0);

    m_path = new QLabel;
    m_path->setWordWrap(true);
    m_path->setTextInteractionFlags(Qt::TextSelectableByMouse);
    infoLayout->addWidget(m_path, 0, 1);

    m_openExternally = new QToolButton;
    m_openExternally->setStyleSheet(QStringLiteral("border: none;"));
    m_openExternally->setIcon(QIcon::fromTheme(QStringLiteral("file-zoom-in")));
    m_openExternally->setToolTip(i18n("Open with default viewer"));
    m_openExternally->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    infoLayout->addWidget(m_openExternally, 0, 2);
    m_openExternally->hide();
    connect(m_openExternally, &QToolButton::clicked, this, &PreviewWidget::openExternally);

    m_dateTimeLabel = new QLabel(i18n("Date/Time:"));
    m_dateTimeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    infoLayout->addWidget(m_dateTimeLabel, 1, 0);

    m_date = new QLabel;
    m_date->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_date->setWordWrap(true);
    infoLayout->addWidget(m_date, 1, 1, 1, 2);

    auto *coordinatesLabel = new QLabel(i18n("Coordinates:"));
    coordinatesLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    infoLayout->addWidget(coordinatesLabel, 2, 0);

    m_coordinates = new QLabel;
    m_coordinates->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_coordinates->setWordWrap(true);
    infoLayout->addWidget(m_coordinates, 2, 1, 1, 2);

    m_preview = new ImagePreview;
    layout->addWidget(m_preview);

    m_matchString[KGeoTag::NotMatched] = i18n("read from file");
    m_matchString[KGeoTag::ExactMatch] = i18n("exact match");
    m_matchString[KGeoTag::InterpolatedMatch] = i18n("interpolated match");
    m_matchString[KGeoTag::ManuallySet] = i18n("manually set");
}

void PreviewWidget::setImage(const QModelIndex &index)
{
    m_currentIndex = index;
    m_currentImage = index.isValid() ? index.data(KGeoTag::PathRole).toString() : QString();

    if (m_currentImage.isEmpty()) {
        m_openExternally->hide();
        m_path->clear();
        m_date->clear();
        m_coordinates->clear();
        m_preview->setImage(QModelIndex());
        return;
    }

    m_openExternally->show();
    m_path->setText(m_currentImage);
    if (m_cameraClockDeviation == 0) {
        m_date->setText(index.data(KGeoTag::DateRole).value<QDateTime>()
                            .toString(m_locale->dateTimeFormat()));
    } else {
        m_date->setText(
            index.data(KGeoTag::DateRole).value<QDateTime>()
                           .addSecs(m_cameraClockDeviation)
                           .toString(m_locale->dateTimeFormat()));
    }

    const auto coordinates = index.data(KGeoTag::CoordinatesRole).value<Coordinates>();
    if (coordinates.isSet()) {
        m_coordinates->setText(i18n("<p>Position: %1; Altitude: %2<br/>(%3)</p>",
            m_formatter->format(coordinates),
            m_formatter->alt(coordinates),
            m_matchString.value(index.data(KGeoTag::MatchTypeRole).value<KGeoTag::MatchType>())));
    } else {
        m_coordinates->setText(i18n("<i>No coordinates set</i>"));
    }

    m_preview->setImage(index);
}

QString PreviewWidget::currentImage() const
{
    return m_currentImage;
}

void PreviewWidget::reload()
{
    setImage(m_currentIndex);
}

void PreviewWidget::setCameraClockDeviation(int deviation)
{
    if (deviation == 0) {
        m_dateTimeLabel->setText(i18n("Date/Time:"));
    } else {
        m_dateTimeLabel->setText(i18n("Date/Time\n(corrected):"));
    }

    m_cameraClockDeviation = deviation;
    reload();
}

void PreviewWidget::openExternally()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_currentImage));
}
