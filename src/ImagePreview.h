// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

// Qt includes
#include <QLabel>
#include <QImage>

// Qt classes
class QTimer;

class ImagePreview : public QLabel
{
    Q_OBJECT

public:
    explicit ImagePreview(QWidget *parent = nullptr);
    void setImage(const QModelIndex &index);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void setScaledPreview();

private: // Variables
    QImage m_currentImage;
    QTimer *m_smoothTimer;

};

#endif // IMAGEPREVIEW_H
