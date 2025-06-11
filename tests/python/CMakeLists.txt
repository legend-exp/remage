file(
  GLOB _file_list
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml *.py)

# copy them to the build area
foreach(_file ${_file_list})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME python/file-merging COMMAND ${PYTHONPATH} ./test_file_merging.py)
set_tests_properties(python/file-merging PROPERTIES LABELS mt)

add_test(NAME python/cli COMMAND "${REMAGE_PYEXE}" -q --help)

add_test(NAME python/pytest-wrapper COMMAND ${PYTHONPATH} -m pytest -vvv
                                            ${CMAKE_CURRENT_SOURCE_DIR})
