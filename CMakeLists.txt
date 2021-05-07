# SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tobias.leupold@gmx.de>
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16.0)
project(kgeotag LANGUAGES CXX VERSION 1.0.0)

set(QT_MIN_VERSION "5.15.2")
set(KF5_DEP_VERSION "5.79.0")
set(KExiv2_MIN_VERSION "5.0.0")
set(Marble_MIN_VERSION "0.28.0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

find_package(ECM ${KF5_DEP_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)

# Find Qt
find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Widgets Network REQUIRED)
set(CMAKE_AUTOMOC ON)
add_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_DEPRECATED_WARNINGS
    -DQT_STRICT_ITERATORS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050B00
)

# Find KDE
find_package(KF5 ${KF5_DEP_VERSION} COMPONENTS CoreAddons I18n XmlGui ConfigWidgets Crash REQUIRED)
find_package(KF5KExiv2 ${KExiv2_MIN_VERSION} REQUIRED)

# Find Marble
find_package(Marble ${Marble_MIN_VERSION} REQUIRED)

# Generate version.h
add_custom_target(
    UpdateVersion ALL
    COMMAND ${CMAKE_COMMAND} -DBASE_DIR=${CMAKE_SOURCE_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                             -DPROJECT_VERSION="${PROJECT_VERSION}"
                             -P ${CMAKE_SOURCE_DIR}/cmake/UpdateVersion.cmake
    COMMENT "Updating version header"
    BYPRODUCTS ${CMAKE_BINARY_DIR}/version.h
)

# Source files
set(main_ROOT ${CMAKE_SOURCE_DIR}/src)
set(kgeotag_SOURCES
    ${main_ROOT}/main.cpp
    ${main_ROOT}/SharedObjects.cpp
    ${main_ROOT}/Settings.cpp
    ${main_ROOT}/MainWindow.cpp
    ${main_ROOT}/PreviewWidget.cpp
    ${main_ROOT}/MapWidget.cpp
    ${main_ROOT}/ImagePreview.cpp
    ${main_ROOT}/SettingsDialog.cpp
    ${main_ROOT}/FixDriftWidget.cpp
    ${main_ROOT}/GpxEngine.cpp
    ${main_ROOT}/ElevationEngine.cpp
    ${main_ROOT}/BookmarksList.cpp
    ${main_ROOT}/BookmarksWidget.cpp
    ${main_ROOT}/CoordinatesDialog.cpp
    ${main_ROOT}/CoordinatesFormatter.cpp
    ${main_ROOT}/RetrySkipAbortDialog.cpp
    ${main_ROOT}/ImagesModel.cpp
    ${main_ROOT}/ImagesListView.cpp
    ${main_ROOT}/ImagesListFilter.cpp
    ${main_ROOT}/Coordinates.cpp
    ${main_ROOT}/AutomaticMatchingWidget.cpp
    ${main_ROOT}/MimeHelper.cpp
    ${main_ROOT}/ImagesLayer.cpp
    ${main_ROOT}/TracksLayer.cpp
    ${main_ROOT}/MapCenterInfo.cpp
    ${main_ROOT}/GeoDataModel.cpp
    ${main_ROOT}/TracksListView.cpp
    ${main_ROOT}/TrackWalker.cpp
)

# Build the executable
add_executable(kgeotag ${kgeotag_SOURCES})
add_dependencies(kgeotag UpdateVersion)
target_link_libraries(kgeotag
    Qt5::Widgets
    Qt5::Network
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    KF5::ConfigWidgets
    KF5::Crash
    KF5::KExiv2
    Marble
)

# Installation

install(TARGETS kgeotag ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_install_icons(
    ICONS icons/16-apps-kgeotag.png
          icons/22-apps-kgeotag.png
          icons/32-apps-kgeotag.png
          icons/48-apps-kgeotag.png
          icons/64-apps-kgeotag.png
          icons/128-apps-kgeotag.png
    DESTINATION ${KDE_INSTALL_ICONDIR})

install(FILES "timezones/timezones.png"
              "timezones/timezones.json"
        DESTINATION "${KDE_INSTALL_DATADIR}/kgeotag")

install(FILES kgeotagui.rc DESTINATION "${KDE_INSTALL_KXMLGUI5DIR}/kgeotag")
install(PROGRAMS org.kde.kgeotag.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kgeotag.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
