# illustrative plots of the vertex confinement modes: two BEGe detectors
# stacked in a liquid argon volume, sampled in a series of ways.

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# the simulations write their output here
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/output)

# generate the GDML file
add_test(NAME confinement-plots/gen-gdml COMMAND ${PYTHONPATH} make_geometry.py --output
                                                 gdml/bege-stack.gdml)
set_tests_properties(confinement-plots/gen-gdml PROPERTIES LABELS "extra;val" FIXTURES_SETUP
                                                           confinement-plots-gdml-fixture)

# one simulation per confinement mode
set(_cases union-detectors lar-cylinder-intersection lar-cylinder-subtraction detector-surface
           detector-surface-depth)

foreach(_case ${_cases})
  add_test(NAME confinement-plots/gen-output-${_case}
           COMMAND ${REMAGE_PYEXE} -g gdml/bege-stack.gdml -w -o output/${_case}.lh5 --
                   macros/${_case}.mac)

  set_tests_properties(
    confinement-plots/gen-output-${_case}
    PROPERTIES LABELS "extra;val" FIXTURES_SETUP confinement-plots-output-fixture
               FIXTURES_REQUIRED confinement-plots-gdml-fixture)
endforeach()

# assemble the figure with all cases side by side
add_test(NAME confinement-plots/figure COMMAND ${PYTHONPATH} ./make_figure.py --indir output
                                               --output confinement.output.pdf)

set_tests_properties(confinement-plots/figure PROPERTIES LABELS "extra;val" FIXTURES_REQUIRED
                                                         confinement-plots-output-fixture)
