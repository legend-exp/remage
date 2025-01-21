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

set(_macros pos-hdf5 pos-lh5 kin-lh5 kin-hdf5 pos-kin-lh5)

foreach(_mac ${_macros})
  add_test(NAME vertex/${_mac} COMMAND ${PYTHONPATH} run-vtx.py ${REMAGE_PYEXE} ${_mac} 1)
  set_tests_properties(vertex/${_mac} PROPERTIES LABELS extra FIXTURES_REQUIRED vertex-input-fixture)

  # note: only 4 threads available on GitHub actions.
  add_test(NAME vertex-mt/${_mac} COMMAND ${PYTHONPATH} run-vtx.py ${REMAGE_PYEXE} ${_mac} 4)
  set_tests_properties(vertex-mt/${_mac} PROPERTIES LABELS "mt;extra" FIXTURES_REQUIRED
                                                    vertex-input-fixture)
endforeach()
