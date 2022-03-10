cmake_minimum_required(VERSION 3.8)
project(03-optics)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage REQUIRED COMPONENTS GDML)

add_executable(03-optics main.cc)
target_link_libraries(03-optics PUBLIC RMG::remage)
