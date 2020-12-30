# Look for version info from Git

macro(get_version_info)

    # save current tag or branch in PROJECT_TAG
    execute_process(COMMAND bash -c "git describe --tags"
        OUTPUT_VARIABLE _git_revision
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(_git_revision STREQUAL "")
        message(WARNING "Cannot determine ${CMAKE_PROJECT_NAME} revision, is the .git directory there? Did you fetch tags from remote?")
        set(PROJECT_VERSION "unknown")
        set(PROJECT_VERSION_MAJOR "unknown")
    else()
        if (${_git_revision} MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)")
            string(REGEX REPLACE "v([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)?" "\\1;\\2;\\3;\\4" _matches ${_git_revision})
            list(GET _matches 0 PROJECT_VERSION_MAJOR)
            list(GET _matches 1 PROJECT_VERSION_MINOR)
            list(GET _matches 2 PROJECT_VERSION_PATCH)
            list(GET _matches 3 PROJECT_VERSION_EXTRA)
            set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}${PROJECT_VERSION_EXTRA}")
        else()
            message(WARNING "Could not determine major/minor/patch fields from version string '${_git_revision}'")
            set(PROJECT_VERSION "unknown")
            set(PROJECT_VERSION_MAJOR "unknown")
        endif()
    endif()

    message(STATUS "${CMAKE_PROJECT_NAME} version: " ${PROJECT_VERSION})
endmacro()
