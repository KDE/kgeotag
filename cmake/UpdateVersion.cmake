# SPDX-FileCopyrightText: 2018-2021 Tobias Leupold <tl at stonemx dot de>
# SPDX-FileCopyrightText: 2012-2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
#
# SPDX-License-Identifier: BSD-3-Clause

if (NOT DEFINED BASE_DIR)
    message(FATAL_ERROR "UpdateVersion.cmake: BASE_DIR not set. "
                        "Please supply base working directory!")
endif()

# Version header locations
set(VERSION_H_IN        ${BASE_DIR}/src/version.h.in)
set(VERSION_H_CURRENT   ${CMAKE_BINARY_DIR}/version.h.current)
set(VERSION_H_GENERATED ${CMAKE_BINARY_DIR}/version.h)

if (EXISTS "${BASE_DIR}/.git" AND EXISTS "${VERSION_H_IN}")
    # We're not working on a release tarball and have to generate a version.h
    message(STATUS "Generating version.h from git")
    include(${CMAKE_CURRENT_LIST_DIR}/GitDescription.cmake)
    git_get_description(VERSION GIT_ARGS)
    if (NOT VERSION)
        set(VERSION "unknown")
    endif()
else()
    # We're compiling a release tarball and generate version.h
    # from the CMakeLists.txt project version
    message(STATUS "Generating version.h from CMakeLists.txt")
    set(VERSION "${PROJECT_VERSION}")
endif()

message(STATUS "Updating version information to ${VERSION}")
# write version info to a temporary file
configure_file(${VERSION_H_IN} ${VERSION_H_CURRENT})
# update info if changed
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${VERSION_H_CURRENT} ${VERSION_H_GENERATED})
# make sure info doesn't get stale
file(REMOVE ${VERSION_H_CURRENT})
