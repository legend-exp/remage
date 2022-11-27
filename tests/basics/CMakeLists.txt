# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml gdml/*.xml)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros print-volumes.mac vis-co60.mac)
set(_macros_extra run-2nbb.mac vis-2nbb.mac)

foreach(_mac ${_macros} ${_macros_extra})
  add_test(NAME basics/${_mac} COMMAND remage-cli -g gdml/main.gdml -- macros/${_mac})
endforeach()

list(TRANSFORM _macros_extra PREPEND "basics/")
set_tests_properties(${_macros_extra} PROPERTIES LABELS extra)
