cmake_minimum_required(VERSION 3.8)
project(01-gdml)

find_package(remage REQUIRED)

add_executable(01-gdml main.cpp)

target_link_libraries(01-gdml PUBLIC remage)
