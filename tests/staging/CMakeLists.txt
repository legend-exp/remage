file(
  GLOB _file_list
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml *.py)

# copy them to the build area
foreach(_file ${_file_list})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# run python tests using pytest
file(
  GLOB _pytest_files
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  test_*.py)

foreach(_pytest_file ${_pytest_files})
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${_pytest_file}" _pytest_defs
       REGEX "^[ \t]*def[ \t]+test_[A-Za-z0-9_]+\\(")

  foreach(_pytest_def ${_pytest_defs})
    string(REGEX REPLACE "^[ \t]*def[ \t]+(test_[A-Za-z0-9_]+)\\(.*$" "\\1" _pytest_name
                         "${_pytest_def}")
    string(REPLACE ".py" "" _pytest_module "${_pytest_file}")

    set(_ctest_name "staging/${_pytest_module}/${_pytest_name}")
    set(_pytest_nodeid "${_pytest_file}::${_pytest_name}")

    add_test(NAME ${_ctest_name} COMMAND ${PYTHONPATH} -m pytest -s -vvv ${_pytest_nodeid})

    set_tests_properties(${_ctest_name} PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                                                   LABELS "mt;extra;val;val-only")
  endforeach()
endforeach()
