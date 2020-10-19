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

// Local includes
#include "MainWindow.h"
#include "version.h"

// KDE includes
#include <KLocalizedString>
#include <KAboutData>

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QDate>
#include <QLocale>

int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain("kgeotag");
    QApplication application(argc, argv);

    // About data

    KAboutData aboutData;

    aboutData.setComponentName(QStringLiteral("kgeotag"));
    aboutData.setDisplayName(i18n("KGeoTag"));
    aboutData.setVersion(VERSION_STRING);
    aboutData.setShortDescription(i18n("Photo geotagging program"));
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(i18n("Copyright (C) 2020 Tobias Leupold"));
    aboutData.setBugAddress(
        QStringLiteral("https://invent.kde.org/tleupold/kgeotag/-/issues").toUtf8());
    aboutData.setHomepage(QStringLiteral("https://invent.kde.org/tleupold/kgeotag"));
    aboutData.setOrganizationDomain(QStringLiteral("invent.kde.org").toUtf8());

    aboutData.addAuthor(i18n("Tobias Leupold"), i18n("Maintainer"),
                        QStringLiteral("tobias.leupold@gmx.de"));

    KAboutData::setApplicationData(aboutData);

    // Create the main window
    MainWindow mainWindow;
    mainWindow.show();

    // Run the program
    return application.exec();
}
