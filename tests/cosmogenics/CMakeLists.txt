add_subdirectory(surface)
add_subdirectory(MUSUN)
add_subdirectory(NeutronCapture)
add_subdirectory(muon)

set(_tests surface-cosmogenic MUSUN NeutronCapture)

foreach(_t ${_tests})
  add_test(NAME cosmogenics/test-${_t}-build
           COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --config "$<CONFIG>" --target
                   test-${_t})
  set_tests_properties(cosmogenics/test-${_t}-build PROPERTIES LABELS "extra;val" FIXTURES_SETUP
                                                               test-${_t}-fixture)

  add_test(
    NAME cosmogenics/${_t}
    WORKING_DIRECTORY $<TARGET_FILE_DIR:test-${_t}>
    COMMAND test-${_t} run.mac)
  set_tests_properties(cosmogenics/${_t} PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                    test-${_t}-fixture)
endforeach()
