# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml gdml/*.xml)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros complex-volume.mac geometrical.mac native-surface.mac geometrical-and-physical.mac
            geometrical-or-physical.mac native-volume.mac)

foreach(_mac ${_macros})
  add_test(NAME confinement/${_mac} COMMAND remage-cli-cpp -g gdml/geometry.gdml -o test-out.root
                                            -- macros/${_mac})

  add_test(NAME confinement-mt/${_mac} COMMAND remage-cli-cpp -g gdml/geometry.gdml -t 2 -o
                                               test-out.root -- macros/${_mac})
  set_tests_properties(confinement-mt/${_mac} PROPERTIES LABELS mt)

  add_test(NAME confinement-vis/${_mac}
           COMMAND remage-cli-cpp -g gdml/geometry.gdml -o test-out.root -- macros/_vis.mac
                   macros/${_mac} macros/_vis-export.mac)
  set_tests_properties(confinement-vis/${_mac} PROPERTIES LABELS vis)
  set_tests_properties(confinement-vis/${_mac} PROPERTIES SKIP_REGULAR_EXPRESSION
                                                          "couldn't open display")
endforeach()
