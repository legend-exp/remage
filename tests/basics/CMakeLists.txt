# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml gdml/*.xml)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros print-volumes.mac)
set(_macros_extra run-2nbb.mac)
set(_macros_vis vis-co60.mac vis-2nbb.mac)

foreach(_mac ${_macros} ${_macros_extra} ${_macros_vis})
  add_test(NAME basics/${_mac} COMMAND remage-cli-cpp -g gdml/main.gdml -- macros/${_mac})
endforeach()

foreach(_mac ${_macros})
  add_test(NAME basics-mt/${_mac} COMMAND remage-cli-cpp -g gdml/main.gdml -t 2 macros/${_mac})
  set_tests_properties(basics-mt/${_mac} PROPERTIES LABELS mt)
endforeach()

# Geant4 11.0 has navigation bugs leading to flaky CI test runs.
if(Geant4_VERSION VERSION_LESS "11.1.0")
  set_tests_properties("basics/run-2nbb.mac" PROPERTIES DISABLED True)
endif()

if(BxDecay0_THREADSAFE)
  foreach(_mac ${_macros_extra})
    add_test(NAME basics-mt/${_mac} COMMAND remage-cli-cpp -g gdml/main.gdml -t 2 macros/${_mac})
    set_tests_properties(basics-mt/${_mac} PROPERTIES LABELS "mt extra")
  endforeach()

  # Geant4 11.0 has navigation bugs leading to flaky CI test runs.
  if(Geant4_VERSION VERSION_LESS "11.1.0")
    set_tests_properties("basics-mt/run-2nbb.mac" PROPERTIES DISABLED True)
  endif()
endif()

list(TRANSFORM _macros_extra PREPEND "basics/")
set_tests_properties(${_macros_extra} PROPERTIES LABELS extra)

list(TRANSFORM _macros_vis PREPEND "basics/")
set_tests_properties(${_macros_vis} PROPERTIES LABELS vis)
set_tests_properties(${_macros_vis} PROPERTIES SKIP_REGULAR_EXPRESSION "couldn't open display")

# further specific tests.

# expect two overlaps from this prepared geometry.
add_test(NAME basics/overlaps.mac COMMAND remage-cli-cpp -- macros/overlaps.mac)
set_tests_properties(basics/overlaps.mac PROPERTIES PASS_REGULAR_EXPRESSION
                                                    "GeomVol1002.*GeomVol1002")
