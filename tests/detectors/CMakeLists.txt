# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* dumps/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME detectors/gen-output COMMAND ${PYTHONPATH} ./run_sims_det.py ${REMAGE_PYEXE})
set_tests_properties(detectors/gen-output PROPERTIES LABELS extra FIXTURES_SETUP
                                                     det-output-fixture)

add_test(NAME detectors/test-output COMMAND ${PYTHONPATH} test_output.py)
set_tests_properties(detectors/test-output PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                      det-output-fixture)
