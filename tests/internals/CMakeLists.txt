# executable for converting remage output HDF5 with ntuples to LH5 tables
add_executable(test-random EXCLUDE_FROM_ALL test-random.cc)
target_link_libraries(test-random PUBLIC remage)
add_test(NAME internals/build-test-random COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}"
                                                  --config "$<CONFIG>" --target test-random)

set(_run_types Tasking MT Serial)

foreach(_run_type ${_run_types})
  add_test(NAME internals/random-${_run_type} COMMAND test-random ${_run_type})
  set_tests_properties(internals/random-${_run_type} PROPERTIES LABELS extra DEPENDS
                                                                output/build-test-visit-hdf5)
endforeach()
