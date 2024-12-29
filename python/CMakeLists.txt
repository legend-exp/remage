# List here manually all source files. Using GLOB is bad, see:
# https://cmake.org/cmake/help/latest/command/file.html?highlight=Note#filesystem

set(_r ${PROJECT_SOURCE_DIR})

set(PYTHON_SOURCES ${_r}/cmake/cpp_config.py.in ${_r}/python/remage/__init__.py
                   ${_r}/python/remage/cli.py ${_r}/pyproject.toml)

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
  COMMAND ${VENV_DIR}/bin/python -m pip -q install --no-warn-script-location uv
  COMMENT "Configuring Python virtual environment")

add_custom_target(python-virtualenv DEPENDS ${VENV_DIR}/bin/uv)

# install the remage wrapper package into the virtual environment with uv
# (build area), including dependencies for running tests
# NOTE: when uv/pip installs the package and creates the executable for the cli,
# it hardcodes the path to the current python executable (e.g. the one of the
# virtualenv) in the script's shebang
add_custom_command(
  OUTPUT ${VENV_DIR}/bin/remage
  COMMAND
    cp
    ${CMAKE_CURRENT_BINARY_DIR}/cpp_config.build.py # now we want to use the cpp_config for the build area
    ${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_config.py
  COMMAND ${VENV_DIR}/bin/python -m uv -q pip install --reinstall "${CMAKE_SOURCE_DIR}[test]"
  DEPENDS python-virtualenv ${PYTHON_SOURCES}
  COMMENT "Installing remage Python wrapper into the virtual environment")

add_custom_target(remage-cli ALL DEPENDS ${VENV_DIR}/bin/remage)

# store the path to the remage executable, needed later in tests (that must work in the build area)
set_target_properties(remage-cli PROPERTIES PYEXE_PATH ${VENV_DIR}/bin/remage)

# install section

# install the package into the install prefix with the existing uv installation
add_custom_command(
  OUTPUT ${CMAKE_INSTALL_PREFIX}/bin/remage
  COMMAND
    cp
    ${CMAKE_CURRENT_BINARY_DIR}/cpp_config.install.py # now we want to use the cpp_config for the install area
    ${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_config.py
  COMMAND ${VENV_DIR}/bin/python -m uv -q pip install --reinstall --prefix ${CMAKE_INSTALL_PREFIX}
          ${CMAKE_SOURCE_DIR})

add_custom_target(
  install-remage-cli
  DEPENDS ${CMAKE_INSTALL_PREFIX}/bin/remage
  COMMENT "Installing remage Python wrapper")

# hack the install process to also install the remage wrapper
install(
  CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install-remage-cli)"
)
