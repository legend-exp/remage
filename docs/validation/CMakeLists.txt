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

# create an (empty) build directory to satisfy the install rule below if the validation suite
# has not been built yet.
file(MAKE_DIRECTORY ${SPHINX_BUILD})

# find and copy docs image files
file(
  GLOB SPHINX_IMAGES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  _img/*)

file(MAKE_DIRECTORY ${SPHINX_SOURCE}/_img)
foreach(_file ${SPHINX_IMAGES})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/_img COPYONLY)
endforeach()

# collect the plots (and text dumps) produced by the test suite, i.e. all files matching the common
# *.output.* naming scheme. this happens at build time, since those files only exist after the
# tests have been run.
# note: the tests producing these files must be labeled "val"!
set(COPY_TESTS_IMAGES
    ${CMAKE_COMMAND} -DTESTS_BINARY_DIR=${CMAKE_BINARY_DIR}/tests -DIMG_DIR=${SPHINX_SOURCE}/_img
    -P ${PROJECT_SOURCE_DIR}/cmake/CopyTestImages.cmake)

# configure conf.py
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${SPHINX_SOURCE}/conf.py @ONLY)

get_target_property(PYTHONPATH python-virtualenv PYTHONPATH)

# Nice named targets so we can run the job easily. they always re-run: Sphinx keeps its own
# incremental build cache, and the set of test plots is only known at build time anyway.
add_custom_target(
  sphinx-validation
  COMMAND ${COPY_TESTS_IMAGES}
  COMMAND ${PYTHONPATH} -m sphinx --fail-on-warning -q -b html ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${SPHINX_SOURCE}
  DEPENDS remage-cli
  COMMENT "Generating validation report with Sphinx")

add_custom_target(
  sphinx-validation-latex
  COMMAND ${COPY_TESTS_IMAGES}
  COMMAND ${PYTHONPATH} -m sphinx --fail-on-warning -q -b latex ${SPHINX_SOURCE}
          ${SPHINX_BUILD}/latex
  WORKING_DIRECTORY ${SPHINX_SOURCE}
  DEPENDS remage-cli
  COMMENT "Generating LaTeX validation report with Sphinx")

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD}/ DESTINATION ${CMAKE_INSTALL_DOCDIR}/validation)
