set(RMG_GIT_VERSION "v0.0.0")

# try to get the special value replaced by git archive (if this is an archive, after all).
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/.git_archival.txt" GIT_ARCHIVAL)

while(GIT_ARCHIVAL)
  list(POP_FRONT GIT_ARCHIVAL GIT_LINE)
  if(GIT_LINE MATCHES "^describe-name: [^$]")
    string(SUBSTRING ${GIT_LINE} 15 -1 RMG_GIT_VERSION)
  endif()
endwhile()

# try to get the tag ourselves from git describe, if we did not have it set above
# (i.e. we are not in a git-created archive).
if(${RMG_GIT_VERSION} STREQUAL "v0.0.0" AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  find_package(Git)
  if(Git_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --tags --always --match "*[0-9]*"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_OUTPUT
      RESULT_VARIABLE GIT_STATUS
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(GIT_STATUS EQUAL 0)
      set(RMG_GIT_VERSION ${GIT_OUTPUT})
    endif()
  endif()
endif()

# cleanup git describe output to match cmake's expectations.
string(REGEX REPLACE "^v" "" RMG_GIT_VERSION ${RMG_GIT_VERSION})
set(RMG_GIT_VERSION_FULL ${RMG_GIT_VERSION})
string(REGEX REPLACE "-.*$" "" RMG_GIT_VERSION ${RMG_GIT_VERSION})

if(${RMG_GIT_VERSION} STREQUAL "0.0.0")
  message(FATAL_ERROR "failed to find version from git or git archive")
endif()
