cmake_minimum_required(VERSION 3.12)
project(01-gdml)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # stand-alone build.
  find_package(remage REQUIRED COMPONENTS GDML)
endif()

add_executable(01-gdml main.cc)
target_link_libraries(01-gdml PUBLIC RMG::remage)
