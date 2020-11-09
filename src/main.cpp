/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>
    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "MainWindow.h"
#include "SharedObjects.h"
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
    aboutData.setLicense(KAboutLicense::GPL_V3);
    aboutData.setCopyrightStatement(i18n("Copyright (C) 2020 Tobias Leupold"));
    aboutData.setBugAddress(
        QStringLiteral("https://invent.kde.org/graphics/kgeotag/-/issues").toUtf8());
    aboutData.setHomepage(QStringLiteral("https://invent.kde.org/graphics/kgeotag"));
    aboutData.setOrganizationDomain(QStringLiteral("invent.kde.org").toUtf8());

    aboutData.addAuthor(i18n("Tobias Leupold"), i18n("Maintainer"),
                        QStringLiteral("tobias.leupold@gmx.de"));

    KAboutData::setApplicationData(aboutData);

    // Setup all shared objects
    SharedObjects sharedObjects;

    // Create the main window
    MainWindow mainWindow(&sharedObjects);
    mainWindow.show();

    // Run the program
    return application.exec();
}
