file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml *.py *.tex)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME germanium/gen-output COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -w -o output.lh5
                                           -- macros/run.mac)
set_tests_properties(germanium/gen-output PROPERTIES LABELS extra FIXTURES_SETUP output-fixture)

add_test(NAME germanium/bremsstrahlung COMMAND ${PYTHONPATH} ./test_brem.py)

add_test(NAME germanium/e-range COMMAND ${PYTHONPATH} ./test_e_range.py)

foreach(_test germanium/bremsstrahlung germanium/e-range)
  set_tests_properties(${_test} PROPERTIES LABELS extra FIXTURES_REQUIRED output-fixture)
endforeach()
