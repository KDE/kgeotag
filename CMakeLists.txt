cmake_minimum_required(VERSION 3.8.0)
project(kgeotag CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)

# Enable colored output for GCC and Clang when using Ninja
include(${CMAKE_SOURCE_DIR}/cmake/NinjaColors.cmake)

# Find Qt
find_package(Qt5 5.11 COMPONENTS Widgets REQUIRED)
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
find_package(KF5 5.44 REQUIRED COMPONENTS I18n REQUIRED)

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
include_directories(${CMAKE_BINARY_DIR})

# Source files
set(main_ROOT ${CMAKE_SOURCE_DIR}/src)
include_directories(${main_ROOT})
set(main_SOURCES
    ${main_ROOT}/main.cpp
    ${main_ROOT}/MainWindow.cpp
    ${main_ROOT}/Settings.cpp
    ${main_ROOT}/ImagesWidget.cpp
    ${main_ROOT}/PreviewWidget.cpp
    ${main_ROOT}/ImagePreview.cpp
    ${main_ROOT}/MapWidget.cpp
    ${main_ROOT}/ImageCache.cpp
)

# Assemble all sources
set(kgeotag_SOURCES
    ${main_SOURCES}
)

# Build the executable
add_executable(kgeotag ${kgeotag_SOURCES})
add_dependencies(kgeotag UpdateVersion)
target_link_libraries(kgeotag
    Qt5::Widgets
    KF5::I18n
    Marble
)
