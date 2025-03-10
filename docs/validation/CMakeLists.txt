# Sphinx
set(SPHINX_SOURCE ${CMAKE_CURRENT_BINARY_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/_build)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

file(
  GLOB_RECURSE SPHINX_SOURCES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.rst **/*.rst *.md **/*.md)

foreach(_file ${SPHINX_SOURCES})
  get_filename_component(_dir ${_file} DIRECTORY)
  file(MAKE_DIRECTORY ${SPHINX_SOURCE}/${_dir})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/${_dir} COPYONLY)
endforeach()

file(
  GLOB SPHINX_IMAGES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  _img/*)

file(MAKE_DIRECTORY ${SPHINX_SOURCE}/_img)
foreach(_file ${SPHINX_IMAGES})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/_img COPYONLY)
endforeach()

# TODO: now we want to add also images from the test suite. need to
# - make an (explicit) list of images to pick from the test suite
# - convert pdf -> bitmap some of them?

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${SPHINX_SOURCE}/conf.py @ONLY)

add_custom_command(
  OUTPUT ${SPHINX_INDEX_FILE}
  COMMAND ${SPHINX_EXECUTABLE} -q -b html ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${SPHINX_SOURCE}
  DEPENDS ${SPHINX_SOURCES} ${SPHINX_IMAGES} ${DOXYGEN_INDEX_FILE}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in)

# Nice named target so we can run the job easily
add_custom_target(
  sphinx-validation ALL
  DEPENDS ${SPHINX_INDEX_FILE}
  COMMENT "Generating Sphinx validation docs")

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD} DESTINATION ${CMAKE_INSTALL_DOCDIR})
