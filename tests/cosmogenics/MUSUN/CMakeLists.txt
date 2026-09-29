add_executable(test-MUSUN EXCLUDE_FROM_ALL main.cc HPGeTestStand.hh HPGeTestStand.cc)
target_link_libraries(test-MUSUN PUBLIC remage)

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  *.csv *.mac *.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME cosmogenics/MUSUN-procs COMMAND ${PYTHONPATH} -m pytest -s -vvv test_musun_procs.py)
set_tests_properties(cosmogenics/MUSUN-procs PROPERTIES LABELS "extra;mt" PROCESSORS 4)
