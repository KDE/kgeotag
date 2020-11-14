# SPDX-FileCopyrightText: 2012-2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2018-2020 Tobias Leupold <tobias.leupold@gmx.de>
#
# SPDX-License-Identifier: BSD-3-Clause

if (NOT DEFINED BASE_DIR)
    message(FATAL_ERROR "UpdateVersion.cmake: BASE_DIR not set. "
                        "Please supply base working directory!")
endif()

# Version header locations
set(VERSION_H_RELEASE   ${BASE_DIR}/src/version.h)
set(VERSION_H_IN        ${BASE_DIR}/src/version.h.in)
set(VERSION_H_CURRENT   ${CMAKE_BINARY_DIR}/version.h.current)
set(VERSION_H_GENERATED ${CMAKE_BINARY_DIR}/version.h)

if (EXISTS ${VERSION_H_RELEASE})
    # A ready to use version.h has been found, so we're compiling a release tarball
    message(STATUS "Using the released version.h")
else()
    # We're not working on a release tarball and have to generate a version.h
    if (EXISTS "${BASE_DIR}/.git" AND EXISTS "${VERSION_H_IN}")
        include(${CMAKE_CURRENT_LIST_DIR}/GitDescription.cmake)
        git_get_description(VERSION GIT_ARGS)
        if (NOT VERSION)
            set(VERSION "unknown")
        endif()

        message(STATUS "Updating version information to ${VERSION}")
        # write version info to a temporary file
        configure_file(${VERSION_H_IN} ${VERSION_H_CURRENT})
        # update info if changed
        execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                                ${VERSION_H_CURRENT} ${VERSION_H_GENERATED})
        # make sure info doesn't get stale
        file(REMOVE ${VERSION_H_CURRENT})

    else()
        message(SEND_ERROR "The generated file \"version.h\" does not exist!")
        message(AUTHOR_WARNING "When creating a release tarball, be sure to include a version.h "
                               "file including the correct version number!")
    endif()
endif()
