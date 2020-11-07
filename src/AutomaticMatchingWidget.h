/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef AUTOMATICMATCHINGWIDGET_H
#define AUTOMATICMATCHINGWIDGET_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QWidget>

// Local classes
class Settings;

// Qt classes
class QSpinBox;
class QCheckBox;
class QAction;

class AutomaticMatchingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AutomaticMatchingWidget(Settings *settings, QWidget *parent = nullptr);
    bool excludeManuallyTagged() const;
    int exactMatchTolerance() const;
    int maximumInterpolationInterval() const;
    int maximumInterpolationDistance() const;

signals:
    void requestReassignment(KGeoTag::SearchType searchType) const;

private slots:
    void enableMaximumInterpolationInterval(bool state);
    void enableMaximumInterpolationDistance(bool state);
    void saveSettings();

private: // Variables
    Settings *m_settings;

    QSpinBox *m_exactMatchTolerance;
    QCheckBox *m_enableMaximumInterpolationInterval;
    QSpinBox *m_maximumInterpolationInterval;
    QCheckBox *m_enableMaximumInterpolationDistance;
    QSpinBox *m_maximumInterpolationDistance;

    QAction *m_excludeManuallyTagged;

};

#endif // AUTOMATICMATCHINGWIDGET_H
