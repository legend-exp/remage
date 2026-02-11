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
foreach(value prestep poststep average)
  add_test(NAME distances-ge/gen-output-${value}
           COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o test-distance-${value}.lh5
                   --flat-output -- macros/test-ge-distance-${value}.mac)
  set_tests_properties(
    distances-ge/gen-output-${value}
    PROPERTIES LABELS extra FIXTURES_SETUP distance-output-fixture-${value} FIXTURES_REQUIRED
               distance-gdml-fixture)

  add_test(NAME distances-ge/distance-${value} COMMAND ${PYTHONPATH} ./test_ge_distance.py
                                                       test-distance-${value}.lh5 ${value})

  set_tests_properties(distances-ge/distance-${value} PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                                 distance-output-fixture-${value})
endforeach()
