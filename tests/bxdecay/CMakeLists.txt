# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME bxdecay/gen-output COMMAND ${PYTHONPATH} ./run_sims.py ${REMAGE_PYEXE})
set_tests_properties(bxdecay/gen-output PROPERTIES LABELS extra FIXTURES_SETUP
                                                   bxdecay-output-fixture)

add_test(NAME bxdecay/plot-output COMMAND ${PYTHONPATH} plot_decay_modes.py)
set_tests_properties(
  bxdecay/plot-output PROPERTIES LABELS extra FIXTURES_REQUIRED bxdecay-output-fixture ENVIRONMENT
                                 MPLCONFIGDIR=${CMAKE_SOURCE_DIR}/tests)
