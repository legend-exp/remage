# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac macros/*.hdf5 macros/*.lh5 gdml/*.gdml gdml/*.xml)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros vert-hdf5.mac vert-lh5.mac)

foreach(_mac ${_macros})
  add_test(NAME vertex/${_mac} COMMAND remage-cli -g gdml/geometry.gdml -- macros/${_mac})
  set_tests_properties(vertex/${_mac} PROPERTIES LABELS extra)
  add_test(NAME vertex-mt/${_mac} COMMAND remage-cli -g gdml/geometry.gdml -t 2 macros/${_mac})
  set_tests_properties(vertex-mt/${_mac} PROPERTIES LABELS "mt extra")
endforeach()
