# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml gdml/*.json *.py data/*)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME neutrons/gen-output COMMAND ${PYTHONPATH} ./run_sims.py)
set_tests_properties(
  neutrons/gen-output PROPERTIES LABELS "extra;val" FIXTURES_SETUP neutrons-output-fixture
                                 PROCESSORS ${STATS_FACTOR})
add_test(NAME neutrons/plot-spectra COMMAND ${PYTHONPATH} make_spectra_and_pies.py)
set_tests_properties(neutrons/plot-spectra PROPERTIES LABELS "extra;val" FIXTURES_REQUIRED
                                                      neutrons-output-fixture)
add_test(NAME neutrons/plot-argon COMMAND ${PYTHONPATH} argon_endf_comparison.py)
set_tests_properties(neutrons/plot-argon PROPERTIES LABELS "extra;val" FIXTURES_REQUIRED
                                                    neutrons-output-fixture)
add_test(NAME neutrons/plot-gd COMMAND ${PYTHONPATH} gd_endf_comparison.py)
set_tests_properties(neutrons/plot-gd PROPERTIES LABELS "extra;val" FIXTURES_REQUIRED
                                                 neutrons-output-fixture)
add_test(NAME neutrons/show-geom COMMAND ${PYTHONPATH} -m pygeomtools.viewer gdml/geometry.gdml
                                         --scene gdml/scene.json)
set_tests_properties(neutrons/show-geom PROPERTIES LABELS "extra;val" FIXTURES_REQUIRED
                                                   neutrons-output-fixture)
