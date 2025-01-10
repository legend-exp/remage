# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac macros/*.json gdml/*.gdml gdml/*.xml *.py)

# copy them to the build area

foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# generate the GDML file
add_test(NAME distances-ge/gen-gdml COMMAND ${PYTHONPATH} make_ge_gdml.py)
set_tests_properties(distances-ge/gen-gdml PROPERTIES LABELS extra FIXTURES_SETUP
                                                      distance-gdml-fixture)

# test on HPGe containment
add_test(NAME distances-ge/gen-output COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o
                                              test-distance.lh5 -- macros/test-ge-distance.mac)
set_tests_properties(
  distances-ge/gen-output PROPERTIES LABELS extra FIXTURES_SETUP distance-output-fixture
                                     FIXTURES_REQUIRED distance-gdml-fixture)

add_test(NAME distances-ge/distance COMMAND ${PYTHONPATH} ./test_ge_distance.py)

set_tests_properties(distances-ge/distance PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                      distance-output-fixture)
