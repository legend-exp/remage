# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME processes/innerbrem-run COMMAND ${PYTHONPATH} ./innerbrem_run_sims.py
                                              ${REMAGE_PYEXE})
set_tests_properties(processes/innerbrem-run PROPERTIES LABELS extra FIXTURES_SETUP
                                                        innerbrem-output-fixture)

add_test(NAME processes/innerbrem-plot COMMAND ${PYTHONPATH} innerbrem_plot_effect.py)
set_tests_properties(
  processes/innerbrem-plot PROPERTIES LABELS extra FIXTURES_REQUIRED innerbrem-output-fixture
                                      ENVIRONMENT MPLCONFIGDIR=${CMAKE_SOURCE_DIR}/tests)
