include(ProcessorCount)
ProcessorCount(NUM_CORES)

file(
  GLOB _file_list
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml data/* *.py *.yaml)

# copy them to the build area
foreach(_file ${_file_list})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME gespectrum/gen-output COMMAND ${PYTHONPATH} run_sims.py)
set_tests_properties(
  gespectrum/gen-output PROPERTIES LABELS "extra;mt;val;val-only" FIXTURES_SETUP
                                   gespectrum-output-fixture PROCESSORS ${NUM_CORES})

add_test(NAME gespectrum/plot COMMAND ${PYTHONPATH} ./plot_gespectrum.py)
set_tests_properties(gespectrum/plot PROPERTIES LABELS "extra;mt;val;val-only" FIXTURES_REQUIRED
                                                gespectrum-output-fixture)
