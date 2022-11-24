cmake_minimum_required(VERSION 3.8)
project(remage-test-basics)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC remage)

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml gdml/*.xml)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros print-volumes.mac vis-co60.mac)
set(_macros_extra run-2nbb.mac vis-2nbb.mac)

foreach(_mac ${_macros} ${_macros_extra})
  add_test(NAME basics/${_mac}
    COMMAND ${PROJECT_NAME} macros/${_mac}
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endforeach()

string(PREPEND ${_test_names} ${_macros_extra})
set_tests_properties(${_test_names} PROPERTIES LABELS extra)
