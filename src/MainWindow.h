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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Qt includes
#include <QMainWindow>

// Local classes
class Settings;
class ImagesWidget;
class MapWidget;

// Qt classes
class QDockWidget;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *) override;

private: // Functions
    QDockWidget *createDockWidget(const QString &title, QWidget *widget, const QString &objectName);

private: // Variables
    Settings *m_settings;
    ImagesWidget *m_imagesWidget;
    MapWidget *m_mapWidget;

};

#endif // MAINWINDOW_H
