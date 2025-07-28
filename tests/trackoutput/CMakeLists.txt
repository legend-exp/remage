# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME trackoutput/gen-output COMMAND ${PYTHONPATH} run_sims_trackoutput.py)
set_tests_properties(trackoutput/gen-output PROPERTIES LABELS extra FIXTURES_SETUP
                                                       track-output-fixture)

add_test(NAME trackoutput/test-output COMMAND ${PYTHONPATH} test_trackoutput.py)
set_tests_properties(trackoutput/test-output PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                        track-output-fixture)
