cmake_minimum_required(VERSION 3.8)
project(02-hpge)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage)

add_executable(02-hpge main.cc HPGeTestStand.hh HPGeTestStand.cc)
target_link_libraries(02-hpge PUBLIC RMG::remage)
