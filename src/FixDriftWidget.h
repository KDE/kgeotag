// SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#ifndef FIXDRIFTWIDGET_H
#define FIXDRIFTWIDGET_H

// Qt includes
#include <QWidget>
#include <QTimeZone>

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
    bool displayFixed() const;
    QByteArray imagesTimeZoneId() const;
    const QTimeZone &imagesTimeZone() const;
    bool setImagesTimeZone(const QByteArray &id);

signals:
    void imagesTimeZoneChanged();
    void cameraDriftSettingsChanged();

private: // Variables
    QComboBox *m_timeZone;
    QSpinBox *m_driftHours;
    QSpinBox *m_driftMinutes;
    QSpinBox *m_driftSeconds;
    QCheckBox *m_displayFixed;
    QCheckBox *m_save;
    QTimeZone m_imagesTimeZone;

};

#endif // FIXDRIFTWIDGET_H
