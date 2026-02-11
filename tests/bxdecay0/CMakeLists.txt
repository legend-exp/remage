# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME bxdecay0/gen-output COMMAND ${PYTHONPATH} ./run_sims.py ${REMAGE_PYEXE})
set_tests_properties(bxdecay0/gen-output PROPERTIES LABELS extra FIXTURES_SETUP
                                                    bxdecay0-output-fixture)

add_test(NAME bxdecay0/plot-output COMMAND ${PYTHONPATH} plot_decay_modes.py)
set_tests_properties(bxdecay0/plot-output PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                     bxdecay0-output-fixture)
