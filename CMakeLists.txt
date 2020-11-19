cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(remage
    DESCRIPTION "Simulation framework for HPGe-based experiments" 
    LANGUAGES CXX)
set(PROJECT_TARNAME remage)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(STATUS "${CMAKE_PROJECT_NAME} requires an out-of-source build.")
    message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
    message(STATUS " - CMakeCache.txt")
    message(STATUS " - CMakeFiles")
    message(STATUS "Once these files are removed, create a separate directory")
    message(STATUS "and run CMake from there")
    message(FATAL_ERROR "in-source build detected")
endif()

# include path for custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules/)

# get install directories names
include(GNUInstallDirs)
# we prefer just 'lib' over 'lib64'
set(CMAKE_INSTALL_LIBDIR lib)

# Set default build type
include(BuildType)

# Find ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT 6.06 REQUIRED Core Hist Tree Gui Graf MathCore Rint)
if(ROOT_FOUND)
    message(STATUS "Found ROOT - v" ${ROOT_VERSION})
endif()

# Define useful ROOT functions and macros
include(${ROOT_USE_FILE})

# # find BxDecay0
# find_package(BxDecay0 1.0 REQUIRED)
# if(BxDecay0_FOUND)
#     message(STATUS "Found BxDecay0 - v" ${BxDecay0_VERSION})
# endif()

# Find Geant4
option(WITH_GEANT4_UIVIS "Build ${CMAKE_PROJECT_NAME} with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 10.4.3 REQUIRED gdml ui_all vis_all)
else()
  find_package(Geant4 10.4.3 REQUIRED gdml)
endif()
if(Geant4_FOUND)
    message(STATUS "Found Geant4 - v" ${Geant4_VERSION})
endif()

# Define useful Geant4 functions and macros
include(${Geant4_USE_FILE})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# all the code is located under src/
add_subdirectory(src)
# add_subdirectory(data)
# add_subdirectory(doc)

# option(BUILD_TESTS "Build ${CMAKE_PROJECT_NAME} test suite" OFF)
# if(BUILD_TESTS)
#     message(STATUS "Building of test suite enabled")
#     enable_testing()
#     add_subdirectory(test)
# endif()

# get project version infos
include(GetVersionInfo)
get_version_info()

# write useful post-install setup scripts
include(Toolchain)
create_mage_toolchain()

include(Colors)
message(STATUS "${CMAKE_PROJECT_NAME} will be installed under ${CMAKE_INSTALL_PREFIX}")

if(BUILD_TESTS)
    message(STATUS "Run the test suite with 'make test'")
endif()
