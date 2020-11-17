# Look for version info from Git

macro(get_version_info)

    # save current tag or branch in PROJECT_TAG
    execute_process(COMMAND bash -c "git describe --tags"
        OUTPUT_VARIABLE _git_revision
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(_git_revision STREQUAL "")
        message(WARNING "Cannot determine ${CMAKE_PROJECT_NAME} revision, is the .git directory there?")
        set(PROJECT_VERSION "unknown")
        set(PROJECT_VERSION_MAJOR "unknown")
    else()
        set(PROJECT_VERSION ${_git_revision})
        string(REGEX_MATCHALL "^v[0-9]\.[0-9]\.[0-9]-.*$" _matched_list ${PROJECT_VERSION})
        list(LENGTH _matched_list _len)
        if(${_len} EQUAL 3)
            list(GET _matched_list 0 PROJECT_VERSION_MAJOR)
            list(GET _matched_list 1 PROJECT_VERSION_MINOR)
            list(GET _matched_list 2 PROJECT_VERSION_PATCH)
        else()
            message(WARNING "Could not determine major/minor/patch fields from version string " {PROJECT_VERSION})
            set(PROJECT_VERSION_MAJOR "unknown")
        endif()
    endif()

    message(STATUS "${CMAKE_PROJECT_NAME} version: " ${PROJECT_VERSION})

    # write ProjectInfo.hh and install it (anyways)
    configure_file(${CMAKE_SOURCE_DIR}/cmake/ProjectInfo.hh.in ${CMAKE_SOURCE_DIR}/src/io/include/ProjectInfo.hh @ONLY)
    install(FILES ${CMAKE_SOURCE_DIR}/src/io/include/ProjectInfo.hh
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_TARNAME})
endmacro()
