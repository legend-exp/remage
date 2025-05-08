# executable for converting remage output HDF5 with ntuples to LH5 tables
set(_remage_to_lh5 "false")
if(RMG_HAS_HDF5)
  set(_remage_to_lh5 $<TARGET_FILE:remage-to-lh5>)
endif()

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# MAIN OUTPUT TESTS
set(_macros
    ntuple-per-det.mac ntuple-per-det-vol.mac ntuple-per-det-auto.mac ntuple-per-det-vol-auto.mac
    ntuple-single-table.mac ntuple-single-table-with-tracks.mac)

foreach(_mac ${_macros})
  add_test(NAME output/hdf5-${_mac} COMMAND ./run-test-hdf5.sh ${REMAGE_PYEXE} ${PYTHONPATH}
                                            ${_remage_to_lh5} ${_mac})
  add_test(NAME output/root-${_mac} COMMAND ./run-test-root.sh ${REMAGE_PYEXE} ${_mac})
endforeach()

list(TRANSFORM _macros PREPEND "output/hdf5-" OUTPUT_VARIABLE _macros_hdf5)
set_tests_properties(${_macros_hdf5} PROPERTIES LABELS extra DEPENDS output/build-test-visit-hdf5)

# SPECIAL TESTS
add_test(NAME output/th228-chain COMMAND ${PYTHONPATH} run-test-th228-chain.py)
set_tests_properties(output/th228-chain PROPERTIES LABELS extra)
