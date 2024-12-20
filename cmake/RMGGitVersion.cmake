# this is a special value replaced by git archive.
set(RMG_GIT_VERSION "$Format:%(describe:tags=1)$")

# try to get the tag ourselves from git describe, if we did not have it set above
# (i.e. we are not in a git-created archive).
if(RMG_GIT_VERSION MATCHES "Format:")
  set(RMG_GIT_VERSION "v0.0.0")
  find_package(Git)
  if(Git_FOUND)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} describe --tags --match "v*"
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
string(REGEX REPLACE "-.*$" "" RMG_GIT_VERSION ${RMG_GIT_VERSION})
