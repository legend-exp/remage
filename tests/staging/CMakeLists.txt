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

# Heavy physics/performance tests run only in the validation workflow. The
# correctness tests (everything else) run on every push, like the other
# functional tests.
set(_val_only_modules test_physics_regression test_muon_stress test_throughput)

foreach(_pytest_file ${_pytest_files})
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${_pytest_file}" _pytest_defs
       REGEX "^[ \t]*def[ \t]+test_[A-Za-z0-9_]+\\(")

  string(REPLACE ".py" "" _pytest_module "${_pytest_file}")
  list(FIND _val_only_modules "${_pytest_module}" _val_only_idx)
  if(NOT _val_only_idx EQUAL -1)
    set(_pytest_labels "mt;extra;val;val-only")
  else()
    set(_pytest_labels "mt;extra")
  endif()

  foreach(_pytest_def ${_pytest_defs})
    string(REGEX REPLACE "^[ \t]*def[ \t]+(test_[A-Za-z0-9_]+)\\(.*$" "\\1" _pytest_name
                         "${_pytest_def}")

    set(_ctest_name "staging/${_pytest_module}/${_pytest_name}")
    set(_pytest_nodeid "${_pytest_file}::${_pytest_name}")

    add_test(NAME ${_ctest_name} COMMAND ${PYTHONPATH} -m pytest -s -vvv ${_pytest_nodeid})

    set_tests_properties(${_ctest_name} PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                                                   LABELS "${_pytest_labels}")
  endforeach()
endforeach()
