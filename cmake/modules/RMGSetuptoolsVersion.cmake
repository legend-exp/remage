# create the virtual environment with python-venv and install setuptools-scm
option(RMG_VERSION_SCM_USE_VENV "use venv and uv for setuptools-scm" ON)

set(_scm_python_cmd "python")

if(RMG_VERSION_SCM_USE_VENV)
  set(SCM_VENV_DIR ${CMAKE_BINARY_DIR}/scm_venv)

  if(NOT EXISTS "${SCM_VENV_DIR}/initialized")
    execute_process(COMMAND ${Python3_EXECUTABLE} -m venv "${SCM_VENV_DIR}" COMMAND_ERROR_IS_FATAL
                            ANY)
    execute_process(COMMAND ${SCM_VENV_DIR}/bin/python -m pip -q install --no-warn-script-location
                            --upgrade pip setuptools setuptools-scm COMMAND_ERROR_IS_FATAL ANY)
    file(TOUCH "${SCM_VENV_DIR}/initialized")
  endif()
  set(_scm_python_cmd "${SCM_VENV_DIR}/bin/python")
endif()

# try to get the version from setuptools-scm.
execute_process(
  COMMAND ${_scm_python_cmd} -m setuptools_scm
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_OUTPUT
  RESULT_VARIABLE GIT_STATUS
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(GIT_STATUS EQUAL 0)
  set(RMG_GIT_VERSION ${GIT_OUTPUT})
else()
  message(FATAL_ERROR "failed to find version using setuptools-scm")
endif()

# cleanup setuptools-scm describe output to match cmake's expectations.
set(RMG_GIT_VERSION_FULL ${RMG_GIT_VERSION})
string(REGEX REPLACE "\\.dev.*$" "" RMG_GIT_VERSION ${RMG_GIT_VERSION})
