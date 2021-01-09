/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FIXDRIFTWIDGET_H
#define FIXDRIFTWIDGET_H

// Qt includes
#include <QWidget>

// Qt classes
class QComboBox;
class QSpinBox;
class QCheckBox;

class FixDriftWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FixDriftWidget(QWidget *parent = nullptr);
    int cameraClockDeviation() const;
    bool save() const;
    QByteArray imagesTimeZoneId() const;

signals:
    void imagesTimeZoneChanged();

private: // Variables
    QComboBox *m_timeZone;
    QSpinBox *m_cameraClockDeviation;
    QCheckBox *m_save;

};

#endif // FIXDRIFTWIDGET_H
