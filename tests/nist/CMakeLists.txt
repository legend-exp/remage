file(
  GLOB _file_list
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml data/* *.py)

# copy them to the build area
foreach(_file ${_file_list})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME nist/gen-output COMMAND ${PYTHONPATH} run_e_sims.py)
set_tests_properties(nist/gen-output PROPERTIES LABELS extra FIXTURES_SETUP output-fixture)

add_test(NAME nist/e-range COMMAND ${PYTHONPATH} ./test_electron_interactions.py)

foreach(_test nist/e-range)
  set_tests_properties(${_test} PROPERTIES LABELS extra FIXTURES_REQUIRED output-fixture
                                           ENVIRONMENT MPLCONFIGDIR=${CMAKE_SOURCE_DIR}/tests)
endforeach()
