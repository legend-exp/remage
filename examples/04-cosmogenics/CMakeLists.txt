cmake_minimum_required(VERSION 3.8)
project(04-surface-cosmogenic)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage)

add_executable(04-surface-cosmogenic main.cc HPGeTestStand.hh HPGeTestStand.cc)
target_link_libraries(04-surface-cosmogenic PUBLIC RMG::remage)
