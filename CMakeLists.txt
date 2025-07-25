# SPDX-FileCopyrightText: 2020-2025 Tobias Leupold <tl@stonemx.de>
#
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16.0)
project(kgeotag LANGUAGES CXX VERSION 1.8.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

# Local CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Set appropriate needed versions Qt and KF

set(QT_MIN_VERSION "6.0")
set(KF_MIN_VERSION "6.0")
set(KExiv2_MIN_VERSION "5.1.0")
set(Marble_MIN_VERSION "24.11.70")
set(KDE_COMPILERSETTINGS_LEVEL "6.0")

# Find ECM
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
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
    -DQT_STRICT_ITERATORS
    -DQT_NO_CONTEXTLESS_CONNECT
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x060000
)

# Find KDE
find_package(KF6 ${KF_MIN_VERSION}
    COMPONENTS CoreAddons I18n XmlGui ConfigWidgets Crash DocTools ColorScheme
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

# Generate debugMode.h according to the requested CMAKE_BUILD_TYPE
if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Enabling extra checks for CMAKE_BUILD_TYPE=Debug mode")
    set(DEBUG_MODE ON)
endif()
configure_file(${CMAKE_SOURCE_DIR}/src/debugMode.h.in ${CMAKE_BINARY_DIR}/debugMode.h)

# Build the executable
add_executable(kgeotag ${kgeotag_SOURCES})
add_dependencies(kgeotag UpdateVersion)
add_subdirectory(src)
target_link_libraries(kgeotag
    PRIVATE
    Qt6::Widgets
    Qt6::Network
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
    KF6::ConfigWidgets
    KF6::Crash
    KF6::ColorScheme
    KExiv2Qt6
    Marble
)

# Resources
qt6_add_resources(kgeotag_SOURCES ${CMAKE_SOURCE_DIR}/kgeotag.qrc)

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
