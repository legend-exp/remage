# get the output name of the remage-cli target (set in src/CMakeLists.txt)
get_target_property(REMAGE_CPP_OUTPUT_NAME remage-cli-cpp OUTPUT_NAME)

# construct the full path to the built executable
set(REMAGE_CPP_EXE_PATH "${CMAKE_BINARY_DIR}/${REMAGE_CPP_OUTPUT_NAME}")

# configure cpp_utils.py.in with the dynamically derived path
configure_file("${PROJECT_SOURCE_DIR}/cmake/cpp_utils.py.in"
               "${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_utils.py" @ONLY)

# create the virtual environment with python-venv
# also install the uv package manager
set(VENV_DIR "${CMAKE_BINARY_DIR}/python_venv")

add_custom_command(
  OUTPUT "${VENV_DIR}/bin/activate"
  COMMAND "${PYTHON_EXECUTABLE}" -m venv "${VENV_DIR}"
  COMMAND "${VENV_DIR}/bin/python" -m pip install --upgrade pip
  COMMAND "${VENV_DIR}/bin/python" -m pip install uv)

add_custom_target(
  python-virtualenv
  DEPENDS "${VENV_DIR}/bin/activate"
  COMMENT "Configuring Python virtual environment in ${VENV_DIR}")

# install the remage wrapper package into the virtual environment with uv
add_custom_command(
  OUTPUT "${VENV_DIR}/bin/remage"
  COMMAND "${VENV_DIR}/bin/python" -m uv pip install "${CMAKE_SOURCE_DIR}"
  DEPENDS python-virtualenv)

add_custom_target(
  remage-cli ALL
  DEPENDS "${VENV_DIR}/bin/remage"
  COMMENT "Installing remage Python wrapper into the virtual environment")

# store the path to the remage executable, needed later in tests
set_target_properties(remage-cli PROPERTIES PYEXE_PATH "${VENV_DIR}/bin/remage")

# install section

# construct the full path to the installed executable
set(REMAGE_CPP_EXE_PATH
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${REMAGE_CPP_OUTPUT_NAME}")

configure_file("${PROJECT_SOURCE_DIR}/cmake/cpp_utils.py.in"
               "${CMAKE_CURRENT_SOURCE_DIR}/remage/cpp_utils.py" @ONLY)

# install the package into the install prefix with the existing uv installation
add_custom_command(
  OUTPUT "${CMAKE_INSTALL_PREFIX}/bin/remage"
  COMMAND "${VENV_DIR}/bin/python" -m uv pip install --prefix "${CMAKE_INSTALL_PREFIX}"
          "${CMAKE_SOURCE_DIR}")

add_custom_target(
  install-remage-cli
  DEPENDS "${CMAKE_INSTALL_PREFIX}/bin/remage"
  COMMENT "Installing remage Python wrapper")

# hack the install process to also install the remage wrapper
install(
  CODE "execute_process(COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install-remage-cli)"
)
