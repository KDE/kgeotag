# SPDX-FileCopyrightText: 2012-2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
# SPDX-FileCopyrightText: 2020 Tobias Leupold <tl at stonemx dot de>
#
# SPDX-License-Identifier: BSD-3-Clause

# - GIT_GET_DESCRIPTION ( description_variable [GIT_ARGS arg1 ...] [SEND_ERROR] )
# This macro calls git describe to obtain a concise description of the current
# git repository, i.e. the most recent tag, together with thea "distance" to
# that tag and the short hash of the current commit.
#
# The description will be written into 'description_variable'. If an error
# occurred, the variable will be set to '-NOTFOUND' unless SEND_ERROR is set.
# In this case the a SEND_ERROR will be issued and generation will be skipped.
#
# Additional arguments to "git described" can be specified after the keyword
# GIT_ARGS.
#
# Example invocation:
#  GIT_GET_DESCRIPTION ( PROJECT_VERSION GIT_ARGS --dirty )
#  message ( STATUS "Project is at revision ${PROJECT_VERSION}." )
#
# Assuming the project has been previously tagged with "1.0", there have been 2
# commits since, and the working copy has been changed, this example could print
# the following text:
#  -- Project is at revision 1.0-2-g58a35d9-dirty.
#
# Depends on CMakeParseArguments.
##

include(CMakeParseArguments)

## git_get_description ( <DESCRIPTION_VARIABLE> [GIT_ARGS <arg1>..<argN>] [SEND_ERROR] )
# Write the result of "git describe" into the variable DESCRIPTION_VARIABLE.
# Additional arguments to git describe ( e.g. --dirty ) can be passed using the keyword GIT_ARGS.
# If SEND_ERROR is set, execution is immediately stopped when an error occurs.

function(git_get_description DESCVAR)
    cmake_parse_arguments(ggd "SEND_ERROR" "GIT_ARGS" "" "${ARGN}")
    if (SEND_ERROR)
        set(severity SEND_ERROR)
    else()
        set(severity WARNING)
    endif()

    find_package(Git QUIET)

    if (NOT GIT_FOUND)
        message(${severity} "git_get_description: could not find package git!")
        set(${DESCVAR} "-NOTFOUND" PARENT_SCOPE)
        return()
    endif()

    # Check if any tag has been set
    execute_process(COMMAND "${GIT_EXECUTABLE}" tag -l
                    WORKING_DIRECTORY "${BASE_DIR}"
                    OUTPUT_VARIABLE git_output
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (git_output STREQUAL "")
        message(STATUS "git_get_description: No tags set yet, can't determine version. Using the "
                       "current commit hash.")
        execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
                        WORKING_DIRECTORY "${BASE_DIR}"
                        OUTPUT_VARIABLE git_output
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(${DESCVAR} "commit-${git_output}" PARENT_SCOPE)
        return()
    endif()

    execute_process(COMMAND "${GIT_EXECUTABLE}" describe --dirty ${ggd_GIT_ARGS}
                    WORKING_DIRECTORY "${BASE_DIR}"
                    RESULT_VARIABLE git_result
                    OUTPUT_VARIABLE git_output
                    ERROR_VARIABLE  git_error
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (NOT git_result EQUAL 0)
        message(${severity} "git_get_description: error during execution of git describe!")
        message(${severity} "Error was: ${git_error}")
        set(${DESCVAR} "-NOTFOUND" PARENT_SCOPE)
        return()
    endif()

    string(REGEX REPLACE "^v" "" git_output ${git_output})
    set(${DESCVAR} "${git_output}" PARENT_SCOPE)
endfunction()
