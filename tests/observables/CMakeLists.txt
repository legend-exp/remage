# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac config/*.yaml *.py gdml/*)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# create the gdml
add_test(NAME observables-ge/gen-gdml COMMAND ${PYTHONPATH} make_gdml.py)

set_tests_properties(observables-ge/gen-gdml PROPERTIES LABELS extra FIXTURES_SETUP
                                                        observables-ge-gdml-fixture)

# visualise it
add_test(NAME observables-ge/vis COMMAND ${REMAGE_PYEXE} macros/vis.mac -g gdml/geometry.gdml -w
                                         -l detail)

set_tests_properties(observables-ge/vis PROPERTIES LABELS "extra;vis" FIXTURES_REQUIRED
                                                   observables-ge-gdml-fixture)
set_tests_properties(observables-ge/vis PROPERTIES SKIP_REGULAR_EXPRESSION "couldn't open display")

## run
add_test(NAME observables-ge/run COMMAND ${PYTHONPATH} ./run_sim.py ${REMAGE_PYEXE})
set_tests_properties(
  observables-ge/run PROPERTIES LABELS "extra" FIXTURES_REQUIRED observables-ge-gdml-fixture
                                FIXTURES_SETUP observables-ge-run-fixture)
## plot some tracks

add_test(NAME observables-ge/tracks-bulk-no-limit
         COMMAND ${PYTHONPATH} ./plot_steps.py out/beta_bulk/step_limits/max_None/stp/out.lh5
                 tracks-bulk-no-limit)
set_tests_properties(observables-ge/tracks-bulk-no-limit
                     PROPERTIES LABELS "extra" FIXTURES_REQUIRED observables-ge-run-fixture)

add_test(NAME observables-ge/tracks-bulk-10um-limit
         COMMAND ${PYTHONPATH} ./plot_steps.py out/beta_bulk/step_limits/max_10/stp/out.lh5
                 tracks-bulk-10um-limit)
set_tests_properties(observables-ge/tracks-bulk-10um-limit
                     PROPERTIES LABELS "extra" FIXTURES_REQUIRED observables-ge-run-fixture)

add_test(NAME observables-ge/observables COMMAND ${PYTHONPATH} ./plot_observables.py 20000
                                                 beta-observables)
set_tests_properties(observables-ge/observables PROPERTIES LABELS "extra" FIXTURES_REQUIRED
                                                           observables-ge-run-fixture)
