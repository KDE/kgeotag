// SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QWidget>
#include <QHash>
#include <QModelIndex>

// Local classes
class SharedObjects;
class CoordinatesFormatter;
class ImagePreview;

// Qt classes
class QLabel;
class QLocale;

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(SharedObjects *sharedObjects, QWidget *parent = nullptr);
    QString currentImage() const;
    void setCameraClockDeviation(int deviation);
    void reload();

public slots:
    void setImage(const QModelIndex &index = QModelIndex());

private: // Variables
    CoordinatesFormatter *m_formatter;
    const QLocale *m_locale;

    ImagePreview *m_preview;

    QLabel *m_path;
    QLabel *m_dateTimeLabel;
    QLabel *m_date;
    QLabel *m_coordinates;
    QHash<KGeoTag::MatchType, QString> m_matchString;
    QString m_currentImage;
    QModelIndex m_currentIndex;
    int m_cameraClockDeviation = 0;

};

#endif // PREVIEWWIDGET_H
