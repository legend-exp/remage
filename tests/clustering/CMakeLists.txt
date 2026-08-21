# effect of the germanium (pre-)clustering settings on CPU time, output file
# size and the reconstructed observables. The geometry, the reboost
# configuration and the plotting helpers are shared with the observables-ge
# test.

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# reuse the geometry, the hit configuration and the plotting helpers of the
# observables-ge test
foreach(_file make_gdml.py plot_observables.py config/hit_config.yaml)
  configure_file(${PROJECT_SOURCE_DIR}/observables/${_file} ${CMAKE_CURRENT_BINARY_DIR}/${_file}
                 COPYONLY)
endforeach()

# the geometry is written here
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gdml)

# create the gdml
add_test(NAME clustering-ge/gen-gdml COMMAND ${PYTHONPATH} make_gdml.py)

set_tests_properties(clustering-ge/gen-gdml PROPERTIES LABELS "extra;val" FIXTURES_SETUP
                                                       clustering-ge-gdml-fixture)

# run the scans over the pre-clustering settings
add_test(NAME clustering-ge/run COMMAND ${PYTHONPATH} ./run_sim.py ${REMAGE_PYEXE})
set_tests_properties(
  clustering-ge/run
  PROPERTIES LABELS
             "extra;val;val-only;mt"
             FIXTURES_REQUIRED
             clustering-ge-gdml-fixture
             FIXTURES_SETUP
             clustering-ge-run-fixture
             PROCESSORS
             ${STATS_FACTOR})

# CPU time, output size and observables vs. the clustering settings
add_test(NAME clustering-ge/observables COMMAND ${PYTHONPATH} ./plot_clustering.py clustering-ge)
set_tests_properties(
  clustering-ge/observables PROPERTIES LABELS "extra;val;val-only;mt" FIXTURES_REQUIRED
                                       clustering-ge-run-fixture)
