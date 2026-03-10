cmake_minimum_required(VERSION 3.12)

add_executable(test-NeutronCapture EXCLUDE_FROM_ALL main.cc HPGeTestStand.hh HPGeTestStand.cc
                                                    IsotopeOutputScheme.cc IsotopeOutputScheme.hh)
target_link_libraries(test-NeutronCapture PUBLIC RMG::remage)

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  *.mac GammaCascades/*.txt)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()
