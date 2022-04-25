# SPDX-FileCopyrightText: 2020-2022 Tobias Leupold <tl at l3u dot de>
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16.0)
project(kgeotag LANGUAGES CXX VERSION 1.2.0)

set(QT_MIN_VERSION "5.11.0")
set(KF5_MIN_VERSION "5.44.0")
set(KExiv2_MIN_VERSION "5.0.0")
set(Marble_MIN_VERSION "21.12.0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

# Local CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Find ECM
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
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
find_package(KF5 ${KF5_MIN_VERSION}
    COMPONENTS CoreAddons I18n XmlGui ConfigWidgets Crash DocTools
    REQUIRED
)

# Find KExiv2
find_package(KF5KExiv2 ${KExiv2_MIN_VERSION} REQUIRED)
if (${KF5KExiv2_FOUND})
    message(STATUS "Found KExiv2 version ${KF5KExiv2_VERSION}")
endif()

# Find Marble
find_package(Marble ${Marble_MIN_VERSION} REQUIRED)
if (${Marble_FOUND})
    message(STATUS "Found Marble version ${Marble_VERSION}")
endif()

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
    PRIVATE
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

# Documentation
kdoctools_create_handbook(
    doc/index.docbook
    INSTALL_DESTINATION "${KDE_INSTALL_DOCBUNDLEDIR}/en"
    SUBDIR kgeotag
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
