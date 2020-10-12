/* Copyright (c) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

// Qt includes
#include <QDialog>
#include <QColor>

// Local classes
class Settings;

// Qt classes
class QPushButton;
class QLabel;
class QSpinBox;
class QComboBox;
class QCheckBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(Settings *settings, QWidget *parent);

protected:
    virtual void accept() override;

private slots:
    void setTrackColor();
    void enableMaximumInterpolationInterval(bool state);

private: // Functions
    void updateTrackColor();

private: // Variables
    Settings *m_settings;

    QColor m_currentTrackColor;
    QPushButton *m_trackColor;
    QLabel *m_trackOpacity;
    QSpinBox *m_trackWidth;
    QComboBox *m_trackStyle;

    QSpinBox *m_exactMatchDeviation;
    QCheckBox *m_enableMaximumInterpolationInterval;
    QSpinBox *m_maximumInterpolationInterval;

};

#endif // SETTINGSDIALOG_H
