# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME combine_tracks/gen-output COMMAND ${PYTHONPATH} ./run_sims.py ${REMAGE_PYEXE})
set_tests_properties(combine_tracks/gen-output PROPERTIES LABELS extra FIXTURES_SETUP
                                                          combine_tracks-output-fixture)

add_test(NAME combine_tracks/plot-output COMMAND ${PYTHONPATH} plot_output.py)
set_tests_properties(
  combine_tracks/plot-output
  PROPERTIES LABELS extra FIXTURES_REQUIRED combine_tracks-output-fixture ENVIRONMENT
             MPLCONFIGDIR=${CMAKE_SOURCE_DIR}/tests)
