/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>
    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FIXDRIFTWIDGET_H
#define FIXDRIFTWIDGET_H

// Qt includes
#include <QWidget>

// Qt classes
class QSpinBox;
class QCheckBox;

class FixDriftWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FixDriftWidget(QWidget *parent = nullptr);
    int deviation() const;
    bool save() const;

private: // Variables
    QSpinBox *m_deviation;
    QCheckBox *m_save;

};

#endif // FIXDRIFTWIDGET_H
