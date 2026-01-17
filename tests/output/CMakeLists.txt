# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* dumps/* gdml/*.gdml gdml/*.xml run-test-*.sh *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# MAIN OUTPUT TESTS
set(_macros
    ntuple-per-det.mac
    ntuple-per-det-vol.mac
    ntuple-per-det-shared.mac
    ntuple-single-table.mac
    ntuple-single-table-with-tracks.mac
    ntuple-optionals.mac
    ntuple-no-stp.mac)

foreach(_mac ${_macros})
  add_test(NAME output/hdf5-${_mac} COMMAND ./run-test-hdf5.sh ${REMAGE_PYEXE} ${PYTHONPATH}
                                            ${_mac})
  add_test(NAME output/root-${_mac} COMMAND ./run-test-root.sh ${REMAGE_PYEXE} ${_mac})

  add_test(NAME output-mt/hdf5-${_mac} COMMAND ./run-test-hdf5.sh ${REMAGE_PYEXE} ${PYTHONPATH}
                                               ${_mac} mt)
  add_test(NAME output-mp/hdf5-${_mac} COMMAND ./run-test-hdf5.sh ${REMAGE_PYEXE} ${PYTHONPATH}
                                               ${_mac} mp)
endforeach()

list(TRANSFORM _macros PREPEND "output/hdf5-" OUTPUT_VARIABLE _macros_hdf5)
set_tests_properties(${_macros_hdf5} PROPERTIES LABELS extra)
list(TRANSFORM _macros PREPEND "output-mt/hdf5-" OUTPUT_VARIABLE _macros_hdf5)
set_tests_properties(${_macros_hdf5} PROPERTIES LABELS "mt;extra" PROCESSORS 2)
list(TRANSFORM _macros PREPEND "output-mp/hdf5-" OUTPUT_VARIABLE _macros_hdf5)
set_tests_properties(${_macros_hdf5} PROPERTIES LABELS "mt;extra" PROCESSORS 2)

# SPECIAL TESTS
add_test(NAME output/th228-chain COMMAND ${PYTHONPATH} run-test-th228-chain.py)
set_tests_properties(output/th228-chain PROPERTIES LABELS extra)
