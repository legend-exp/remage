cmake_minimum_required(VERSION 3.8)
project(06-FullCosmogenics)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage REQUIRED COMPONENTS GDML)

add_executable(06-FullCosmogenics main.cc)
target_link_libraries(06-FullCosmogenics PUBLIC RMG::remage)
