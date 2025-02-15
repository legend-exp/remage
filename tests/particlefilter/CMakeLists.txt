# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/* gdml/*.gdml gdml/*.xml run-test-*.sh run-test-*.py)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

add_test(NAME particlefilter/test-particlefilter
         COMMAND ${PYTHONPATH} ./run-test-particlefilter.py $<TARGET_FILE:remage-cli-cpp>)
set_tests_properties(particlefilter/test-particlefilter PROPERTIES LABELS extra)
