cmake_minimum_required(VERSION 3.8)
project(06-NeutronCapture)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage)

add_executable(06-NeutronCapture main.cc HPGeTestStand.hh HPGeTestStand.cc IsotopeOutputScheme.cc
                                 IsotopeOutputScheme.hh)
target_link_libraries(06-NeutronCapture PUBLIC RMG::remage)
