cmake_minimum_required(VERSION 3.8)
project(TestNeutronCapture)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage)

add_executable(TestNeutronCapture main.cc HPGeTestStand.hh HPGeTestStand.cc IsotopeOutputScheme.cc
                                  IsotopeOutputScheme.hh)
target_link_libraries(TestNeutronCapture PUBLIC RMG::remage)
