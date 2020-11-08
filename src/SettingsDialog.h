/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
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

signals:
    void imagesListsModeChanged() const;

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

    QCheckBox *m_lookupElevationAutomatically;
    QComboBox *m_elevationDataset;

    QComboBox *m_writeMode;
    QCheckBox *m_createBackups;

};

#endif // SETTINGSDIALOG_H
