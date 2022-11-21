find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

# find remage headers
get_target_property(REMAGE_PUBLIC_HEADER_DIR remage INTERFACE_INCLUDE_DIRECTORIES)
file(GLOB REMAGE_PUBLIC_HEADERS ${REMAGE_PUBLIC_HEADER_DIR}/*.hh)

# configure Doxyfile
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR})
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# Doxygen won't create this for us
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

# configure target that runs Doxygen
add_custom_command(
  OUTPUT ${DOXYGEN_INDEX_FILE}
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
  DEPENDS ${DOXYFILE_IN} ${REMAGE_PUBLIC_HEADERS}
  MAIN_DEPENDENCY ${DOXYFILE_OUT}
  COMMENT "Running Doxygen")

add_custom_target(
  doxygen ALL
  DEPENDS ${DOXYGEN_INDEX_FILE}
  COMMENT "Generating docs")

# Sphinx

set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR})
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

configure_file(${SPHINX_SOURCE}/conf.py.in ${SPHINX_SOURCE}/conf.py @ONLY)

add_custom_command(
  OUTPUT ${SPHINX_INDEX_FILE}
  COMMAND
    ${SPHINX_EXECUTABLE} -b html
    # Tell Breathe where to find the Doxygen output
    -Dbreathe_projects.remage=${DOXYGEN_OUTPUT_DIR}/xml ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${SPHINX_SOURCE}/index.rst ${DOXYGEN_INDEX_FILE}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  ${SPHINX_SOURCE}/conf.py.in
  COMMENT "Generating documentation with Sphinx")

# Nice named target so we can run the job easily
add_custom_target(sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD} DESTINATION ${CMAKE_INSTALL_DOCDIR})
