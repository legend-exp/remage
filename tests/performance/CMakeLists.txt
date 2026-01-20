# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME performance/combine-tracks COMMAND ${PYTHONPATH} ./combine_tracks_benchmark.py
                                                 ${REMAGE_PYEXE})
set_tests_properties(performance/combine-tracks PROPERTIES LABELS extra)
