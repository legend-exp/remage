# List here manually all source files. Using GLOB is bad, see:
# https://cmake.org/cmake/help/latest/command/file.html?highlight=Note#filesystem

set(_r ${PROJECT_SOURCE_DIR})

# cmake-format: off
set(PYTHON_SOURCES
    ${_r}/pyproject.toml
    ${_r}/cmake/cpp_config.py.in
    ${_r}/python/remage/__init__.py
    ${_r}/python/remage/find_remage.py
    ${_r}/python/remage/cli.py
    ${_r}/python/remage/ipc.py)
# cmake-format: on

# get the output name of the remage-cli target (set in src/CMakeLists.txt)
get_target_property(REMAGE_CPP_OUTPUT_NAME remage-cli-cpp OUTPUT_NAME)

# 1) construct the full path to the built executable
set(REMAGE_CPP_EXE_PATH ${CMAKE_BINARY_DIR}/src/${REMAGE_CPP_OUTPUT_NAME})

# configure cpp_config.py.in for the build area with the dynamically derived path
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/cpp_config.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/cpp_config.build.py # temporary location
  @ONLY)

# 2) construct the full path to the installed executable
set(REMAGE_CPP_EXE_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${REMAGE_CPP_OUTPUT_NAME})

# configure cpp_config.py.in for the install area
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/cpp_config.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/cpp_config.install.py # temporary location
  @ONLY)

# create the virtual environment with python-venv
# also install the uv package manager
set(VENV_DIR ${CMAKE_BINARY_DIR}/python_venv)

add_custom_command(
  OUTPUT ${VENV_DIR}/bin/uv
  COMMAND ${Python3_EXECUTABLE} -m venv ${VENV_DIR}
  COMMAND ${VENV_DIR}/bin/python -m pip -q install --no-warn-script-location --upgrade pip
  COMMAND ${VENV_DIR}/bin/python -m pip -q install --no-warn-script-location --upgrade uv
  COMMENT "Configuring Python virtual environment")

add_custom_target(python-virtualenv DEPENDS ${VENV_DIR}/bin/uv)

# store the path to the python executable, needed later in tests
set_target_properties(python-virtualenv PROPERTIES PYTHONPATH ${VENV_DIR}/bin/python
                                                   ADDITIONAL_CLEAN_FILES ${VENV_DIR})

# install the remage wrapper package into the virtual environment with uv
# (build area), including dependencies for running tests
# NOTE: when uv/pip installs the package and creates the executable for the cli,
# it hardcodes the path to the current python executable (e.g. the one of the
# virtualenv) in the script's shebang

# need more packages if we also want to use this venv later for tests
if(BUILD_TESTING)
  set(_pkg_install "${CMAKE_SOURCE_DIR}[test]")
else()
  set(_pkg_install ${CMAKE_SOURCE_DIR})
endif()

# now we want to use the cpp_config for the build area
add_custom_command(
  OUTPUT ${VENV_DIR}/bin/remage
  COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cpp_config.build.py
          ${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_config.py
  COMMAND SETUPTOOLS_SCM_PRETEND_VERSION_FOR_REMAGE=${RMG_GIT_VERSION_FULL} ${VENV_DIR}/bin/python
          -m uv pip -q install --reinstall ${_pkg_install}
  COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_config.py
  COMMAND printf -- "-- Installed python wrapper "
  COMMAND sh -c '${VENV_DIR}/bin/python -m uv pip show remage 2>&1 | grep Version'
  DEPENDS python-virtualenv ${PYTHON_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/cpp_config.build.py
  COMMENT "Installing remage Python wrapper into the virtual environment")

add_custom_target(remage-cli ALL DEPENDS ${VENV_DIR}/bin/remage)

# store the path to the remage executable, needed later in tests (that must work in the build area)
set_property(TARGET remage-cli PROPERTY PYEXE_PATH ${VENV_DIR}/bin/remage)
set_property(
  TARGET remage-cli
  PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_config.py"
           "${CMAKE_CURRENT_BINARY_DIR}/cpp_config.build.py")

# install section

set(INSTALL_VENV_DIR ${CMAKE_INSTALL_PREFIX}/share/remage_venv)

# install the package into the install prefix together with a virtualenv
# in this way remage should always run isolated in its environment.
# we want to use the cpp_config for the install area
add_custom_command(
  OUTPUT ${CMAKE_INSTALL_PREFIX}/bin/remage ${CMAKE_INSTALL_PREFIX}/share/remage-py3-none-any.whl
  # prepare venv
  COMMAND ${Python3_EXECUTABLE} -m venv ${INSTALL_VENV_DIR}
  COMMAND ${INSTALL_VENV_DIR}/bin/python -m pip -q install --no-warn-script-location --upgrade pip
  COMMAND ${INSTALL_VENV_DIR}/bin/python -m pip -q install --no-warn-script-location --upgrade uv
  # install our package into this venv
  COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cpp_config.install.py
          ${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_config.py
  COMMAND SETUPTOOLS_SCM_PRETEND_VERSION_FOR_REMAGE=${RMG_GIT_VERSION_FULL}
          ${INSTALL_VENV_DIR}/bin/python -m uv -q pip install --reinstall ${CMAKE_SOURCE_DIR}
  COMMAND ln -fs ${INSTALL_VENV_DIR}/bin/remage ${CMAKE_INSTALL_PREFIX}/bin/remage
  # create and install a user-installable wheel
  COMMAND
    SETUPTOOLS_SCM_PRETEND_VERSION_FOR_REMAGE=${RMG_GIT_VERSION_FULL}
    ${INSTALL_VENV_DIR}/bin/python -m uv build --wheel ${CMAKE_SOURCE_DIR} -o
    ${CMAKE_INSTALL_PREFIX}/share
  COMMAND mv ${CMAKE_INSTALL_PREFIX}/share/remage-*-py3-none-any.whl
          ${CMAKE_INSTALL_PREFIX}/share/remage-py3-none-any.whl
  COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_config.py
  # we want to use the cpp_config for the install area
  DEPENDS ${PYTHON_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/cpp_config.install.py
  WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
  COMMENT "Installing remage Python wrapper")

add_custom_target(install-remage-cli DEPENDS ${CMAKE_INSTALL_PREFIX}/bin/remage)

set_property(
  TARGET install-remage-cli
  PROPERTY ADDITIONAL_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_config.py"
           "${CMAKE_CURRENT_BINARY_DIR}/cpp_config.install.py")

# hack the install process to also install the remage wrapper
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target\
                                      install-remage-cli -- --no-print-directory)")
