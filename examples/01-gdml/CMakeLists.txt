cmake_minimum_required(VERSION 3.8)
project(01-gdml)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage REQUIRED COMPONENTS GDML)

add_executable(01-gdml main.cc)
target_link_libraries(01-gdml PUBLIC RMG::remage)
