# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac macros/*.json gdml/*.gdml gdml/*.xml *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros complex-volume.mac geometrical.mac native-surface.mac geometrical-and-physical.mac
            geometrical-or-physical.mac native-volume.mac)

foreach(_mac ${_macros})
  add_test(NAME confinement/${_mac} COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -o test-out.root
                                            -- macros/${_mac})

  add_test(NAME confinement-mt/${_mac} COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -t 2 -o
                                               test-out.root -- macros/${_mac})
  set_tests_properties(confinement-mt/${_mac} PROPERTIES LABELS mt)

  add_test(NAME confinement-vis/${_mac}
           COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -o test-out.root -- macros/_vis.mac
                   macros/${_mac} macros/_vis-export.mac)
  set_tests_properties(confinement-vis/${_mac} PROPERTIES LABELS vis)
  set_tests_properties(confinement-vis/${_mac} PROPERTIES SKIP_REGULAR_EXPRESSION
                                                          "couldn't open display")
endforeach()

# generate the GDML file
add_test(NAME confinment-ge/gen-gdml COMMAND ${PYTHONPATH} make_ge_array_gdml.py)
set_tests_properties(confinment-ge/gen-gdml PROPERTIES LABELS extra FIXTURES_SETUP
                                                       confine-gdml-fixture)

# test on HPGe containment
add_test(NAME confinment-ge/gen-output COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o
                                               test-confine.lh5 -- macros/test-ge-confine.mac)
set_tests_properties(
  confinment-ge/gen-output PROPERTIES LABELS extra FIXTURES_SETUP confine-output-fixture
                                      FIXTURES_REQUIRED confine-gdml-fixture)

add_test(NAME confinment-ge/relative-fraction COMMAND ${PYTHONPATH} ./test_confinment_ge.py)

set_tests_properties(confinment-ge/relative-fraction PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                                confine-output-fixture)

# test on lar intersection
add_test(NAME confinment-lar/intersection-gen-output
         COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o test-confine-lar-in.lh5 --
                 macros/lar-in.mac)
set_tests_properties(
  confinment-lar/intersection-gen-output
  PROPERTIES LABELS extra FIXTURES_SETUP confine-lar-in-output-fixture FIXTURES_REQUIRED
             confine-gdml-fixture)

add_test(NAME confinment-lar/intersection COMMAND ${PYTHONPATH} ./test_lar_intersection.py)

set_tests_properties(confinment-lar/intersection PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                            confine-lar-in-output-fixture)

# generate subtraction output
add_test(NAME confinment-lar/subtraction-gen-output
         COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o test-confine-lar-out.lh5 --
                 macros/lar-out.mac)

set_tests_properties(
  confinment-lar/subtraction-gen-output
  PROPERTIES LABELS extra FIXTURES_SETUP confine-lar-out-output-fixture FIXTURES_REQUIRED
             confine-gdml-fixture)

# analyse subtraction
add_test(NAME confinment-lar/subtraction COMMAND ${PYTHONPATH} ./test_lar_subtraction.py
                                                 test-confine-lar-out.lh5 lar-sub-check.output.pdf)

set_tests_properties(confinment-lar/subtraction PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                           confine-lar-out-output-fixture)

# generate intersection and subtraction output
add_test(NAME confinment-lar/intersection-and-subtraction-gen-output
         COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o test-confine-lar-in-out.lh5 --
                 macros/lar-in-and-out.mac)

set_tests_properties(
  confinment-lar/intersection-and-subtraction-gen-output
  PROPERTIES LABELS extra FIXTURES_SETUP confine-lar-in-out-output-fixture FIXTURES_REQUIRED
             confine-gdml-fixture)

# analyse intersection and subtraction
add_test(NAME confinment-lar/intersection-and-subtraction
         COMMAND ${PYTHONPATH} ./test_lar_subtraction.py test-confine-lar-in-out.lh5
                 lar-int-and-sub-check.output.pdf)

set_tests_properties(confinment-lar/intersection-and-subtraction
                     PROPERTIES LABELS extra FIXTURES_REQUIRED confine-lar-ibn-out-output-fixture)

# test generic surface sampling methods
add_executable(test-surface-sampler-methods EXCLUDE_FROM_ALL test-surface-sampler-methods.cpp)
target_link_libraries(test-surface-sampler-methods PUBLIC remage Geant4)
