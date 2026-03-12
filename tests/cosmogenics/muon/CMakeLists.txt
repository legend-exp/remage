file(
  GLOB _file_list
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml *.py)

# copy them to the build area
foreach(_file ${_file_list})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# run python tests using pytest
add_test(NAME cosmogenics/muon/pytest-all COMMAND ${PYTHONPATH} -m pytest -s -vvv .)
set_tests_properties(cosmogenics/muon/pytest-all PROPERTIES LABELS "extra;val")
