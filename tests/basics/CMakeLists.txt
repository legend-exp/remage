cmake_minimum_required(VERSION 3.8)
project(remage-test-basics)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(remage REQUIRED COMPONENTS GDML)

add_executable(${PROJECT_NAME} main.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC RMG::remage)

# collect auxiliary files
file(GLOB _aux RELATIVE ${PROJECT_SOURCE_DIR} macros/*.mac gdml/*)

# copy them to the build area
foreach(_file ${_aux})
    configure_file(
        ${PROJECT_SOURCE_DIR}/${_file}
        ${PROJECT_BINARY_DIR}/${_file}
        COPYONLY)
endforeach()

add_custom_target(test
    COMMENT "Running ${PROJECT_NAME} tests..."
    COMMAND ${PROJECT_NAME} macros/print-volumes.mac
    COMMAND ${PROJECT_NAME} macros/run-2nbb.mac
    COMMAND ${PROJECT_NAME} macros/vis-2nbb.mac
    COMMAND ${PROJECT_NAME} macros/vis-co60.mac
    VERBATIM)
