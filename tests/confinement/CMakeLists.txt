cmake_minimum_required(VERSION 3.8)
project(remage-test-confinement)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC remage)

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac gdml/*.gdml)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros
  complex-volume.mac
  geometrical.mac
  native-surface.mac
  geometrical-and-physical.mac
  geometrical-or-physical.mac
  native-volume.mac)

# TODO: labels
foreach(_mac ${_macros})
  add_test(NAME ${_mac} COMMAND ${PROJECT_NAME} macros/${_mac})
endforeach()
