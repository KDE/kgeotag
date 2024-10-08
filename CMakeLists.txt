# SPDX-FileCopyrightText: 2020-2024 Tobias Leupold <tl at stonemx dot de>
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16.0)
project(kgeotag LANGUAGES CXX VERSION 1.6.0)

set(QT_MIN_VERSION "6.0")
set(KF_MIN_VERSION "6.0")
set(KExiv2_MIN_VERSION "5.1.0")
set(Marble_MIN_VERSION "")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

# Create a debugMode.h file according to the requested CMAKE_BUILD_TYPE
if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Enabling extra checks for CMAKE_BUILD_TYPE=Debug mode")
    set(DEBUG_MODE ON)
endif()
configure_file(${CMAKE_SOURCE_DIR}/src/debugMode.h.in ${CMAKE_BINARY_DIR}/debugMode.h)

# Local CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Find ECM
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(KDE_COMPILERSETTINGS_LEVEL ${KF_MIN_VERSION})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)

# Find Qt
find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS Widgets Network REQUIRED)
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
find_package(KF6 ${KF_MIN_VERSION}
    COMPONENTS CoreAddons I18n XmlGui ConfigWidgets Crash DocTools
    REQUIRED
)

# Find KExiv2
find_package(KExiv2Qt6 ${KExiv2_MIN_VERSION} REQUIRED)
if (${KExiv2Qt6_FOUND})
    message(STATUS "Found KExiv2 version ${KExiv2Qt6_VERSION}")
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
    ${main_ROOT}/Logging.cpp
    ${main_ROOT}/DegreesConverter.cpp
)

qt6_add_resources(kgeotag_SOURCES ${CMAKE_SOURCE_DIR}/kgeotag.qrc)

# Build the executable
add_executable(kgeotag ${kgeotag_SOURCES})
add_dependencies(kgeotag UpdateVersion)
target_link_libraries(kgeotag
    PRIVATE
    Qt6::Widgets
    Qt6::Network
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
    KF6::ConfigWidgets
    KF6::Crash
    KExiv2Qt6
    Marble
)

# Documentation
add_subdirectory(doc)

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

install(PROGRAMS org.kde.kgeotag.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kgeotag.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ki18n_install(po)
