# Sphinx
set(SPHINX_SOURCE ${CMAKE_CURRENT_BINARY_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/_build)

# find and copy docs source files
file(
  GLOB_RECURSE SPHINX_SOURCES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.rst **/*.rst *.md **/*.md)

foreach(_file ${SPHINX_SOURCES})
  get_filename_component(_dir ${_file} DIRECTORY)
  file(MAKE_DIRECTORY ${SPHINX_SOURCE}/${_dir})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/${_dir} COPYONLY)
endforeach()

# find and copy docs image files
file(
  GLOB SPHINX_IMAGES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  _img/*)

file(MAKE_DIRECTORY ${SPHINX_SOURCE}/_img)
foreach(_file ${SPHINX_IMAGES})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/_img COPYONLY)
endforeach()

# now we want to add also images from the test suite. need to make an
# (explicit) list of images to pick from the test suite
# TODO: add new images here
# cmake-format: off
set(TESTS_IMAGES
    confinement/native-surface.output.jpeg
    confinement/native-volume.output.jpeg
    confinement/complex-surface.output.jpeg
    confinement/complex-volume.output.jpeg
    confinement/geometrical.output.jpeg
    confinement/geometrical-or-physical.output.jpeg
    confinement/geometrical-and-physical.output.jpeg
    confinement/surface-sample-bounding-box-simple.output.jpeg
    confinement/surface-sample-bounding-box-subtraction.output.jpeg
    confinement/surface-sample-bounding-box-union.output.jpeg
    confinement/vis-surface-box.output.jpeg
    confinement/vis-surface-con.output.jpeg
    confinement/vis-surface-sub.output.jpeg
    confinement/vis-surface-tubby.output.jpeg
    confinement/vis-surface-uni.output.jpeg
    confinement/vis-surface-trd.output.jpeg
    nist/e-range-ge-distributions.output.png
    nist/e-range-ge-vs-estar.output.png
    nist/e-stopping-power-ge-vs-estar.output.png)

# cmake-format: on

# copy them over, but at build time!
set(COPIED_IMGS)
foreach(_file ${TESTS_IMAGES})
  set(_src ${CMAKE_BINARY_DIR}/tests/${_file})
  set(_dest ${CMAKE_CURRENT_BINARY_DIR}/_img/${_file})
  add_custom_command(
    OUTPUT ${_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${_src} ${_dest}
    DEPENDS ${_src})
  list(APPEND COPIED_IMGS ${_dest})
endforeach()

add_custom_target(
  copy_files ALL
  DEPENDS ${COPIED_IMGS}
  COMMENT "Copying images from test suite")

# configure conf.py
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${SPHINX_SOURCE}/conf.py @ONLY)

add_custom_command(
  OUTPUT ${SPHINX_BUILD}/index.html
  COMMAND ${SPHINX_EXECUTABLE} --fail-on-warning -q -b html ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${SPHINX_SOURCE}
  DEPENDS ${SPHINX_SOURCES} ${SPHINX_IMAGES} ${DOXYGEN_INDEX_FILE} ${COPIED_IMGS}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in)

# Nice named target so we can run the job easily
add_custom_target(
  sphinx-validation
  DEPENDS ${SPHINX_BUILD}/index.html
  COMMENT "Generating validation report with Sphinx")

add_custom_command(
  OUTPUT ${SPHINX_BUILD}/latex/remagesvalidationsuite.tex
  COMMAND ${SPHINX_EXECUTABLE} --fail-on-warning -q -b latex ${SPHINX_SOURCE} ${SPHINX_BUILD}/latex
  WORKING_DIRECTORY ${SPHINX_SOURCE}
  DEPENDS ${SPHINX_SOURCES} ${SPHINX_IMAGES} ${DOXYGEN_INDEX_FILE} ${COPIED_IMGS}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in)

# Nice named target so we can run the job easily
add_custom_target(
  sphinx-validation-latex
  DEPENDS ${SPHINX_BUILD}/latex/remagesvalidationsuite.tex
  COMMENT "Generating LaTeX validation report with Sphinx")

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD} DESTINATION ${CMAKE_INSTALL_DOCDIR})
