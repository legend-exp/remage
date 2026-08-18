# illustrative plots of the step pre-clustering: the same events, simulated
# once with and once without clustering of the germanium steps.

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# the geometry is the same as the one used for the confinement plots
configure_file(${PROJECT_SOURCE_DIR}/confinement-plots/make_geometry.py
               ${CMAKE_CURRENT_BINARY_DIR}/make_geometry.py COPYONLY)

# the simulations write their output here
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)

# generate the GDML file
add_test(NAME cluster-plots/gen-gdml COMMAND ${PYTHONPATH} make_geometry.py --output
                                             gdml/bege-stack.gdml)
set_tests_properties(cluster-plots/gen-gdml PROPERTIES LABELS "extra;val" FIXTURES_SETUP
                                                       cluster-plots-gdml-fixture)

# both runs must see the same events: same seed, single thread
foreach(_case cluster nocluster)
  add_test(NAME cluster-plots/gen-output-${_case}
           COMMAND ${REMAGE_PYEXE} --rand-seed 12345 -t 1 -g gdml/bege-stack.gdml -w -o
                   output/${_case}.lh5 -- macros/${_case}.mac)

  set_tests_properties(
    cluster-plots/gen-output-${_case}
    PROPERTIES LABELS "extra;val" FIXTURES_SETUP cluster-plots-output-fixture FIXTURES_REQUIRED
               cluster-plots-gdml-fixture)
endforeach()

# draw the most energetic events, clustered and un-clustered side by side
add_test(NAME cluster-plots/figure COMMAND ${PYTHONPATH} ./make_figure.py --indir output --output
                                           clustering.output.pdf)

set_tests_properties(cluster-plots/figure PROPERTIES LABELS "extra;val" FIXTURES_REQUIRED
                                                     cluster-plots-output-fixture)
