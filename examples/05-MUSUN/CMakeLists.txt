cmake_minimum_required(VERSION 3.8)
project(05-MUSUN)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  # stand-alone build.
  find_package(remage REQUIRED)
endif()

add_executable(05-MUSUN main.cc HPGeTestStand.hh HPGeTestStand.cc)
target_link_libraries(05-MUSUN PUBLIC RMG::remage)
