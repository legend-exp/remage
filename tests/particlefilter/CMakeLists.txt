# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh run-test-*.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME particlefilter/basicfilter COMMAND ${PYTHONPATH} run-test-basicfilter.py)
set_tests_properties(particlefilter/basicfilter PROPERTIES LABELS "extra;flaky")

add_test(NAME particlefilter/killvolume COMMAND ${PYTHONPATH} run-test-killvolume.py)
set_tests_properties(particlefilter/killvolume PROPERTIES LABELS extra)

add_test(NAME particlefilter/keepvolume COMMAND ${PYTHONPATH} run-test-keepvolume.py)
set_tests_properties(particlefilter/keepvolume PROPERTIES LABELS extra)

add_test(NAME particlefilter/killprocess COMMAND ${PYTHONPATH} run-test-killprocess.py)
set_tests_properties(particlefilter/killprocess PROPERTIES LABELS extra)

add_test(NAME particlefilter/keepprocess COMMAND ${PYTHONPATH} run-test-keepprocess.py)
set_tests_properties(particlefilter/keepprocess PROPERTIES LABELS extra)
