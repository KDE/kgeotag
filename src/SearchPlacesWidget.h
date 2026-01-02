// SPDX-FileCopyrightText: 2026 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef SEARCHPLACESWIDGET_H
#define SEARCHPLACESWIDGET_H

// Qt includes
#include <QWidget>

// Local classes
class SharedObjects;

class SearchPlacesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPlacesWidget(SharedObjects *sharedObjects, QWidget *parent = nullptr);

};

#endif // SEARCHPLACESWIDGET_H
