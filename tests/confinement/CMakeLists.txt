cmake_minimum_required(VERSION 3.8)
project(remage-test-confinement)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC remage)
