# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac *.py gdml/*.gdml gdml/*.xml)

set(_remage_from_lh5 "false")
if(RMG_HAS_HDF5)
  set(_remage_from_lh5 $<TARGET_FILE:remage-from-lh5>)
endif()

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME vertex/gen-input COMMAND ${PYTHONPATH} create-test-input.py ${_remage_from_lh5})
set_tests_properties(vertex/gen-input PROPERTIES LABELS extra FIXTURES_SETUP vertex-input-fixture)

set(_macros vert-hdf5.mac vert-lh5.mac)

foreach(_mac ${_macros})
  add_test(NAME vertex/${_mac} COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -- macros/${_mac})
  set_tests_properties(vertex/${_mac} PROPERTIES LABELS extra FIXTURES_SETUP vertex-input-fixture)
  add_test(NAME vertex-mt/${_mac} COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -t 2
                                          macros/${_mac})
  set_tests_properties(vertex-mt/${_mac} PROPERTIES LABELS "mt extra" FIXTURES_SETUP
                                                    vertex-input-fixture)
endforeach()
