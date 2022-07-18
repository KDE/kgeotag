// SPDX-FileCopyrightText: 2020-2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
#include <QCommandLineParser>

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
    aboutData.setCopyrightStatement(i18n("Copyright (C) 2020-2022 Tobias Leupold"));
    aboutData.setHomepage(QStringLiteral("https://kgeotag.kde.org/"));
    aboutData.setOrganizationDomain(QStringLiteral("kde.org").toUtf8());

    aboutData.addAuthor(i18n("Tobias Leupold"), i18n("Maintainer"),
                        QStringLiteral("tl@stonemx.de"));

    KAboutData::setApplicationData(aboutData);

    // Create the command line parser
    QCommandLineParser commandLineParser;
    commandLineParser.addPositionalArgument(i18n("Files/Directories"),
                                            i18n("Files and/or directories to load at startup"),
                                            i18n("[files/directories...]"));
    aboutData.setupCommandLine(&commandLineParser);
    commandLineParser.process(application);
    aboutData.processCommandLine(&commandLineParser);
    auto pathsToLoad = commandLineParser.positionalArguments();

    // Setup all shared objects
    SharedObjects sharedObjects;

    // Create the main window
    auto *mainWindow = new MainWindow(&sharedObjects);
    mainWindow->show();

    // Trigger loading of files and/or directories given on the command line
    if (! pathsToLoad.isEmpty()) {
        mainWindow->addPathsFromCommandLine(pathsToLoad);
    }

    // Run the QApplication
    return application.exec();
}
