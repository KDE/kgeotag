/* Copyright (C) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
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

private: // Functions
    void updateTrackColor();

private: // Variables
    Settings *m_settings;

    QComboBox *m_imageListsMode;
    QCheckBox *m_splitImagesList;

    QSpinBox *m_thumbnailSize;
    QSpinBox *m_previewSize;
    bool m_originalSplitImagesListValue;
    int m_originalThumbnailSizeValue;
    int m_originalPreviewSizeValue;

    QColor m_currentTrackColor;
    QPushButton *m_trackColor;
    QLabel *m_trackOpacity;
    QSpinBox *m_trackWidth;
    QComboBox *m_trackStyle;

    QCheckBox *m_lookupElevation;
    QComboBox *m_elevationDataset;

    QComboBox *m_writeMode;
    QCheckBox *m_createBackups;

};

#endif // SETTINGSDIALOG_H
