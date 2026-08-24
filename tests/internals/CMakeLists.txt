# executable for testing RNG propagation to worker threads.
add_executable(test-random EXCLUDE_FROM_ALL test-random.cc)
target_link_libraries(test-random PUBLIC remage)
add_test(NAME internals/build-test-random COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}"
                                                  --config "$<CONFIG>" --target test-random)
set_tests_properties(internals/build-test-random PROPERTIES FIXTURES_SETUP random-fixture)

set(_run_types Tasking MT Serial)

foreach(_run_type ${_run_types})
  add_test(NAME internals/random-${_run_type} COMMAND test-random ${_run_type})
  set_tests_properties(internals/random-${_run_type} PROPERTIES LABELS extra FIXTURES_REQUIRED
                                                                random-fixture)
endforeach()

# lightweight unit tests for pure helpers (no full run manager).
add_executable(test-tools EXCLUDE_FROM_ALL test-tools.cc)
target_link_libraries(test-tools PUBLIC remage)
add_test(NAME internals/build-test-tools COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}"
                                                 --config "$<CONFIG>" --target test-tools)
set_tests_properties(internals/build-test-tools PROPERTIES FIXTURES_SETUP tools-fixture)

add_test(NAME internals/tools COMMAND test-tools)
set_tests_properties(internals/tools PROPERTIES FIXTURES_REQUIRED tools-fixture)
