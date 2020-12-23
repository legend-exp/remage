cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(remage
    DESCRIPTION "Simulation framework for HPGe-based experiments" 
    LANGUAGES C CXX) # C is needed for GEANT4's HDF5 support
set(PROJECT_TARNAME remage)

if(WIN32)
    message(FATAL_ERROR "${CMAKE_PROJECT_NAME} is not supported on Windows")
endif()

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
include(Colors)

# get install directories names
include(GNUInstallDirs)
# we prefer just 'lib' over 'lib64'
set(CMAKE_INSTALL_LIBDIR lib)

# Set default build type
include(BuildType)

# Find Geant4
find_package(Geant4 10.4.3 REQUIRED)
if(Geant4_FOUND)
    message(STATUS "Found GEANT4 v" ${Geant4_VERSION})
endif()

# check for optional components
find_package(Geant4 QUIET OPTIONAL_COMPONENTS hdf5 usolids multithreading gdml)

if(Geant4_hdf5_FOUND)
    message(STATUS "GEANT4 compiled with HDF5 support - enabling feature")
else()
    message(STATUS "GEANT4 lacks HDF5 support - disabling feature")
endif()

if(Geant4_usolids_FOUND)
    message(STATUS "GEANT4 compiled with VecGeom support - enabling feature")
else()
    message(STATUS "GEANT4 lacks VecGeom support - disabling feature")
endif()

if(Geant4_multithreading_FOUND)
    message(STATUS "GEANT4 compiled with multithreading support - enabling feature")
else()
    message(STATUS "GEANT4 lacks multithreading support - disabling feature")
endif()

if(Geant4_gdml_FOUND)
    message(STATUS "GEANT4 compiled with GDML support - enabling feature")
    set(REMAGE_HAS_GDML 1)
else()
    message(STATUS "GEANT4 lacks GDML support - disabling feature")
    set(REMAGE_HAS_GDML 0)
endif()

# Define useful Geant4 functions and macros
include(${Geant4_USE_FILE})

# Find ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT 6.06 QUIET COMPONENTS Core Tree)
if(ROOT_FOUND)
    message(STATUS "Found ROOT v" ${ROOT_VERSION} ", support enabled")
endif()

option(REMAGE_USE_DECAY0 "Build ${CMAKE_PROJECT_NAME} with BxDecay0 support" ON)
# find BxDecay0
find_package(BxDecay0 1.0 QUIET)
if(BxDecay0_FOUND)
    if(NOT TARGET BxDecay0::BxDecay0_Geant4)
        message(WARNING "BxDecay0 v" ${BxDecay0_VERSION} " found but without Geant4 extension library, recompile with -DBXDECAY0_WITH_GEANT4_EXTENSION. Disabling support")
        set(BxDecay0_FOUND FALSE)
    else()
        message(STATUS "Found BxDecay0 v" ${BxDecay0_VERSION} ", support enabled")
    endif()
else()
    message(STATUS "BxDecay0 not found, support disabled")
endif()

# set minimum C++ standard
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "CMAKE_CXX_STANDARD is c++" ${CMAKE_CXX_STANDARD})

# all the code is located under src/
add_subdirectory(src)
# add_subdirectory(data)
add_subdirectory(doc)
enable_testing()
add_subdirectory(test)

# get project version info from Git
include(GetVersionInfo)
get_version_info()

# write useful post-install setup scripts
include(Toolchain)
create_mage_toolchain()

include(DependencyGraph)
gen_dep_graph(pdf)

message(STATUS "${CMAKE_PROJECT_NAME} install prefix set to ${CMAKE_INSTALL_PREFIX}")
