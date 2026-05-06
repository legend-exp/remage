file(
  GLOB _file_list
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml misc/*.yaml fluka_comparison/*.json *.py)

# copy them to the build area
foreach(_file ${_file_list})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# run python tests using pytest
add_test(NAME cosmogenics/muon/energy-loss COMMAND ${PYTHONPATH} -m pytest -s -vvv
                                                   test_energy_loss.py)
set_tests_properties(cosmogenics/muon/energy-loss PROPERTIES LABELS "extra;val;val-only" TIMEOUT
                                                             1800 PROCESSORS 16)

add_test(NAME cosmogenics/muon/particle-yield COMMAND ${PYTHONPATH} -m pytest -s -vvv
                                                      test_particle_yield.py)
set_tests_properties(cosmogenics/muon/particle-yield PROPERTIES LABELS "extra;val;val-only"
                                                                TIMEOUT 3600 PROCESSORS 16)
