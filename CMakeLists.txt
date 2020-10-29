cmake_minimum_required(VERSION 3.8.0)
project(kgeotag CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic")

find_package(ECM 1.1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)

# Find Qt
find_package(Qt5 5.11 COMPONENTS Widgets Network REQUIRED)
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
find_package(KF5 COMPONENTS CoreAddons I18n XmlGui ConfigWidgets KExiv2 REQUIRED)

# Find Marble
find_package(Marble 0.28 REQUIRED)

# Generate version.h
add_custom_target(
    UpdateVersion ALL
    COMMAND ${CMAKE_COMMAND} -DBASE_DIR=${CMAKE_SOURCE_DIR} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
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
    ${main_ROOT}/ImageCache.cpp
    ${main_ROOT}/MainWindow.cpp
    ${main_ROOT}/ImagesList.cpp
    ${main_ROOT}/PreviewWidget.cpp
    ${main_ROOT}/MapWidget.cpp
    ${main_ROOT}/ImagePreview.cpp
    ${main_ROOT}/SettingsDialog.cpp
    ${main_ROOT}/FixDriftWidget.cpp
    ${main_ROOT}/GpxEngine.cpp
    ${main_ROOT}/ImageItem.cpp
    ${main_ROOT}/ElevationEngine.cpp
    ${main_ROOT}/BookmarksList.cpp
    ${main_ROOT}/BookmarksWidget.cpp
    ${main_ROOT}/CoordinatesDialog.cpp
    ${main_ROOT}/CoordinatesFormatter.cpp
    ${main_ROOT}/RetrySkipAbortDialog.cpp
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

install(PROGRAMS org.kde.kgeotag.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.kgeotag.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
