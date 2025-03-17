# collect auxiliary files
file(
  GLOB _files
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml gdml/*.xml)

# copy them to the build area
foreach(_file ${_files})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME basics/macro-substitution
         COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -w --macro-substitutions ENERGY=1000
                 GENERATOR=GPS -- macros/run.mac)

# this will fail because the user did not specify all variables to be substituted
add_test(NAME basics/macro-substitution-missing COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -s
                                                        ENERGY=1000 -w -- macros/run.mac)
set_tests_properties(basics/macro-substitution-missing PROPERTIES WILL_FAIL TRUE)
