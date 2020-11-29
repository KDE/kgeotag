/* SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "MainWindow.h"
#include "SharedObjects.h"
#include "version.h"

// KDE includes
#include <KCrash>
#include <KLocalizedString>
#include <KAboutData>

// Qt includes
#include <QApplication>
#include <QDebug>
#include <QDate>
#include <QLocale>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    KLocalizedString::setApplicationDomain("kgeotag");
    KCrash::initialize();

    // About data

    KAboutData aboutData;

    aboutData.setComponentName(QStringLiteral("kgeotag"));
    aboutData.setDisplayName(i18n("KGeoTag"));
    aboutData.setVersion(VERSION_STRING);
    aboutData.setShortDescription(i18n("Photo geotagging program"));
    aboutData.setLicense(KAboutLicense::GPL_V3);
    aboutData.setCopyrightStatement(i18n("Copyright (C) 2020 Tobias Leupold"));
    aboutData.setHomepage(QStringLiteral("https://invent.kde.org/graphics/kgeotag"));
    aboutData.setOrganizationDomain(QStringLiteral("kde.org").toUtf8());

    aboutData.addAuthor(i18n("Tobias Leupold"), i18n("Maintainer"),
                        QStringLiteral("tobias.leupold@gmx.de"));

    KAboutData::setApplicationData(aboutData);

    // Setup all shared objects
    SharedObjects sharedObjects;

    // Create the main window
    auto *mainWindow = new MainWindow(&sharedObjects);
    mainWindow->show();

    // Run the program
    return application.exec();
}
