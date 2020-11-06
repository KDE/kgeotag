/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

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

class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(SharedObjects *sharedObjects, QWidget *parent = nullptr);
    QString currentImage() const;

public slots:
    void setImage(const QModelIndex &index = QModelIndex());

private: // Variables
    CoordinatesFormatter *m_formatter;

    ImagePreview *m_preview;

    QLabel *m_path;
    QLabel *m_date;
    QLabel *m_coordinates;
    QHash<KGeoTag::MatchType, QString> m_matchString;
    QString m_currentImage;

};

#endif // PREVIEWWIDGET_H
