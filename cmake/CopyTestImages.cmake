# Collect the plots (and text dumps) produced by the validation test suite and copy them into the
# Sphinx source tree, preserving the directory layout below tests/. Run at build time (not at
# configure time), since these files only exist after `ctest -L val` has been run.
#
# Only files following the common output naming scheme (*.output.*) are picked up, so that
# intermediate/debug files produced by the tests do not end up in the report.
#
# Expects TESTS_BINARY_DIR and IMG_DIR to be passed with -D.

file(
  GLOB_RECURSE _files
  RELATIVE ${TESTS_BINARY_DIR}
  ${TESTS_BINARY_DIR}/*.output.png ${TESTS_BINARY_DIR}/*.output.jpg
  ${TESTS_BINARY_DIR}/*.output.jpeg ${TESTS_BINARY_DIR}/*.output.pdf
  ${TESTS_BINARY_DIR}/*.output.svg ${TESTS_BINARY_DIR}/*.output.txt)

foreach(_file ${_files})
  get_filename_component(_dir ${_file} DIRECTORY)
  file(COPY ${TESTS_BINARY_DIR}/${_file} DESTINATION ${IMG_DIR}/${_dir})
endforeach()

list(LENGTH _files _n)
message(STATUS "Copied ${_n} files from the test suite to ${IMG_DIR}")
