# collect auxiliary files
file(
  GLOB _files
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml gdml/*.xml *.sh)

# copy them to the build area
foreach(_file ${_files})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME basics/macro-substitution
         COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -o none --macro-substitutions ENERGY=1000
                 GENERATOR=GPS -- macros/run.mac)

# this will fail because the user did not specify all variables to be substituted
add_test(NAME basics/macro-substitution-missing COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -s
                                                        ENERGY=1000 -o none -- macros/run.mac)
set_tests_properties(basics/macro-substitution-missing PROPERTIES WILL_FAIL TRUE)

# this will fail because the xml file is broken
add_test(NAME basics/invalid-xml
         COMMAND ${REMAGE_PYEXE} -g gdml/geometry_invalid_xml.gdml -o none --macro-substitutions
                 ENERGY=1000 GENERATOR=GPS -- macros/run.mac)
set_tests_properties(basics/invalid-xml PROPERTIES WILL_FAIL TRUE)

# this will fail because the xml file is broken, expect both processes to be SIGTERM'd.
add_test(NAME basics-mp/invalid-xml
         COMMAND ${REMAGE_PYEXE} -g gdml/geometry_invalid_xml.gdml -o none -P 2
                 --macro-substitutions ENERGY=1000 GENERATOR=GPS -- macros/run.mac)
set_tests_properties(
  basics-mp/invalid-xml
  PROPERTIES PASS_REGULAR_EXPRESSION "exited with signal SIGTERM.*exited with signal SIGTERM"
             LABELS "mp")

add_test(NAME basics-mp/exit COMMAND ./run-mp-test-exit.sh ${REMAGE_PYEXE})
set_tests_properties(
  basics-mp/exit PROPERTIES PASS_REGULAR_EXPRESSION "SIGTERM.*SIGABRT|SIGABRT.*SIGTERM" LABELS
                            "mp;extra")

# verify passing a seed via CLI is handled and logged
add_test(NAME basics/rand-seed
         COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -o none --rand-seed 123
                 --macro-substitutions ENERGY=1000 GENERATOR=GPS -- macros/run.mac)
set_tests_properties(basics/rand-seed PROPERTIES PASS_REGULAR_EXPRESSION
                                                 "CLHEP::HepRandom seed changed to: 123")
