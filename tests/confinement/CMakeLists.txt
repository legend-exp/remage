# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac macros/*.json gdml/*.gdml gdml/*.xml *.py *.tex)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros
    complex-volume.mac
    complex-surface.mac
    geometrical.mac
    native-surface.mac
    geometrical-and-physical.mac
    geometrical-or-physical.mac
    native-volume.mac)

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
add_test(NAME confinement-ge/gen-gdml COMMAND ${PYTHONPATH} make_ge_array_gdml.py)
set_tests_properties(confinement-ge/gen-gdml PROPERTIES LABELS extra FIXTURES_SETUP
                                                        confine-gdml-fixture)

# test on HPGe containment
add_test(NAME confinement-ge/gen-output COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o
                                                test-confine.lh5 -- macros/test-ge-confine.mac)
set_tests_properties(
  confinement-ge/gen-output PROPERTIES LABELS extra FIXTURES_SETUP confine-output-fixture
                                       FIXTURES_REQUIRED confine-gdml-fixture)

add_test(NAME confinement-ge/relative-fraction COMMAND ${PYTHONPATH} ./test_confinement_ge.py)

set_tests_properties(confinement-ge/relative-fraction PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                                 confine-output-fixture)

# test on lar intersection
add_test(NAME confinement-lar/intersection-gen-output
         COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o test-confine-lar-in.lh5 --
                 macros/lar-in.mac)
set_tests_properties(
  confinement-lar/intersection-gen-output
  PROPERTIES LABELS extra FIXTURES_SETUP confine-lar-in-output-fixture FIXTURES_REQUIRED
             confine-gdml-fixture)

add_test(NAME confinement-lar/intersection COMMAND ${PYTHONPATH} ./test_lar_intersection.py)

set_tests_properties(confinement-lar/intersection PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                             confine-lar-in-output-fixture)

# generate subtraction output
add_test(NAME confinement-lar/subtraction-gen-output
         COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o test-confine-lar-out.lh5 --
                 macros/lar-out.mac)

set_tests_properties(
  confinement-lar/subtraction-gen-output
  PROPERTIES LABELS extra FIXTURES_SETUP confine-lar-out-output-fixture FIXTURES_REQUIRED
             confine-gdml-fixture)

# analyse subtraction
add_test(NAME confinement-lar/subtraction
         COMMAND ${PYTHONPATH} ./test_lar_subtraction.py test-confine-lar-out.lh5
                 lar-sub-check.output.pdf)

set_tests_properties(confinement-lar/subtraction PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                            confine-lar-out-output-fixture)

# generate intersection and subtraction output
add_test(NAME confinement-lar/intersection-and-subtraction-gen-output
         COMMAND ${REMAGE_PYEXE} -g gdml/ge-array.gdml -w -o test-confine-lar-in-out.lh5 --
                 macros/lar-in-and-out.mac)

set_tests_properties(
  confinement-lar/intersection-and-subtraction-gen-output
  PROPERTIES LABELS extra FIXTURES_SETUP confine-lar-in-out-output-fixture FIXTURES_REQUIRED
             confine-gdml-fixture)

# analyse intersection and subtraction
add_test(NAME confinement-lar/intersection-and-subtraction
         COMMAND ${PYTHONPATH} ./test_lar_subtraction.py test-confine-lar-in-out.lh5
                 lar-int-and-sub-check.output.pdf)

set_tests_properties(confinement-lar/intersection-and-subtraction
                     PROPERTIES LABELS extra FIXTURES_REQUIRED confine-lar-in-out-output-fixture)

# test generic surface sampling methods
add_executable(test-surface-sampler-methods EXCLUDE_FROM_ALL test-surface-sampler-methods.cpp)
target_link_libraries(test-surface-sampler-methods PUBLIC remage)
if(BxDecay0_FOUND)
  target_link_libraries(test-surface-sampler-methods PRIVATE BxDecay0::BxDecay0_Geant4)
endif()

add_test(NAME confinement-surface/test-surface-sampler-methods-build
         COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --config "$<CONFIG>" --target
                 test-surface-sampler-methods)

set_tests_properties(confinement-surface/test-surface-sampler-methods-build
                     PROPERTIES LABELS extra FIXTURES_SETUP test-surface-sampler-methods-fixture)

# add the tests
set(_surface_tests
    test-points-subtraction
    test-points-basic
    test-points-union
    test-intersections-basic
    test-intersections-subtraction
    test-intersections-union
    test-containment-subtraction
    test-containment-union)

foreach(_test ${_surface_tests})

  add_test(NAME confinement-surface/${_test} COMMAND ./test-surface-sampler-methods ${_test})

  set_tests_properties(
    confinement-surface/${_test} PROPERTIES LABELS extra FIXTURES_REQUIRED
                                            test-surface-sampler-methods-fixture)

  if(_test MATCHES "test-points-.*")
    set_tests_properties(
      confinement-surface/${_test} PROPERTIES LABELS "extra;vis" SKIP_REGULAR_EXPRESSION
                                              "couldn't open display")
  endif()

endforeach()

# now run some simulations and test the outputs

# generate the GDML file
add_test(NAME confinement-surface/gen-gdml COMMAND ${PYTHONPATH} make_basic_solids.py)
set_tests_properties(confinement-surface/gen-gdml PROPERTIES LABELS extra FIXTURES_SETUP
                                                             confine-surface-gdml-fixture)

# visualise
set(_solids tubby box con uni sub trd)

foreach(det ${_solids})

  add_test(
    NAME confinement-surface/gen-vis-macro-${det}
    COMMAND /bin/sh -c
            "sed 's/\${det}/${det}/g' macros/vis-surface.mac > macros/vis-surface-${det}.mac")

  set_tests_properties(
    confinement-surface/gen-vis-macro-${det}
    PROPERTIES LABELS extra FIXTURES_SETUP confine-surface-vis-macro-${det}-fixture
               FIXTURES_REQUIRED confine-surface-gdml-fixture)

  add_test(NAME confinement-surface/${det}-vis
           COMMAND ${REMAGE_PYEXE} macros/vis-surface-${det}.mac macros/_vis-export.mac -g
                   gdml/simple-solids.gdml -w -l detail)

  set_tests_properties(
    confinement-surface/${det}-vis PROPERTIES LABELS "extra;vis" FIXTURES_REQUIRED
                                              confine-surface-vis-macro-${det}-fixture)
  set_tests_properties(confinement-surface/${det}-vis PROPERTIES SKIP_REGULAR_EXPRESSION
                                                                 "couldn't open display")
endforeach()

# run simulations and check
foreach(det ${_solids})

  add_test(
    NAME confinement-surface/gen-macro-${det}
    COMMAND /bin/sh -c
            "sed 's/\${det}/${det}/g' macros/gen-surface.mac > macros/gen-surface-${det}.mac")

  set_tests_properties(
    confinement-surface/gen-macro-${det}
    PROPERTIES LABELS extra FIXTURES_SETUP confine-surface-macro-${det}-fixture FIXTURES_REQUIRED
               confine-surface-gdml-fixture)

  add_test(NAME confinement-surface/${det}-gen
           COMMAND ${REMAGE_PYEXE} macros/gen-surface-${det}.mac -g gdml/simple-solids.gdml -w -l
                   detail -o test-confine-surface-${det}-out.lh5)

  set_tests_properties(
    confinement-surface/${det}-gen
    PROPERTIES LABELS extra FIXTURES_SETUP confine-surface-${det} FIXTURES_REQUIRED
               confine-surface-macro-${det}-fixture)

  add_test(NAME confinement-surface/relative-fractions-${det}
           COMMAND ${PYTHONPATH} ./test_basic_surface.py ${det})

  set_tests_properties(confinement-surface/relative-fractions-${det}
                       PROPERTIES LABELS extra FIXTURES_REQUIRED confine-surface-${det})

endforeach()
