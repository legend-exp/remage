cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(
  remage
  VERSION 0.1.0
  DESCRIPTION "Simulation framework for HPGe-based experiments"
  LANGUAGES C CXX) # C is needed for Geant4's HDF5 support

message(STATUS "remage version ${CMAKE_PROJECT_VERSION}")

if(WIN32)
  message(FATAL_ERROR "remage is not supported on Windows")
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(STATUS "remage requires an out-of-source build.")
  message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
  message(STATUS " - CMakeCache.txt")
  message(STATUS " - CMakeFiles")
  message(STATUS "Once these files are removed, create a separate directory")
  message(STATUS "and run CMake from there")
  message(FATAL_ERROR "in-source build detected")
endif()

# include path for custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules/)
include(Colors)

# get install directories names
include(GNUInstallDirs)
# we prefer just 'lib' over 'lib64'
set(CMAKE_INSTALL_LIBDIR lib)

# Add uninstall target if required
if(NOT TARGET uninstall)
  configure_file(cmake/cmake_uninstall.cmake.in "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
                 @ONLY)

  add_custom_target(
    uninstall
    COMMAND "${CMAKE_COMMAND}" -P "${PROJECT_BINARY_DIR}/cmake_uninstall.cmake"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}")
endif()

# Only report new/updated install files
set(CMAKE_INSTALL_MESSAGE LAZY)

# Only allow installs relative to CMAKE_INSTALL_PREFIX
set(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION ON)

# export compile commands, e.g. for use by clang-tidy.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type
include(BuildType)

# Copy .clang-tidy file to build dir to ensure clang-tidy always picks up the correct one
# no matter where out build dir is relative to the source dir
if(EXISTS "${PROJECT_SOURCE_DIR}/.clang-tidy")
  configure_file("${PROJECT_SOURCE_DIR}/.clang-tidy" "${PROJECT_BINARY_DIR}/.clang-tidy" COPYONLY)
endif()

# Find dependencies
set(RMG_G4_MINIMUM_VERSION 11.0.3)
set(RMG_ROOT_MINIMUM_VERSION 6.06)
set(RMG_BXDECAY0_MINIMUM_VERSION 1.0.10)

# Find Geant4
find_package(Geant4 ${RMG_G4_MINIMUM_VERSION} REQUIRED)
if(Geant4_FOUND)
  message(STATUS "Found Geant4 v" ${Geant4_VERSION})
endif()

# check for optional components
find_package(Geant4 QUIET OPTIONAL_COMPONENTS hdf5 usolids multithreaded gdml ui_all vis_all)

if(Geant4_hdf5_FOUND)
  message(STATUS "Geant4 compiled with HDF5 support - enabling feature")
  list(APPEND g4_components hdf5)
  list(APPEND remage_components HDF5)
  # need to find HDF5 with CXX again, as Geant4 does not initialize this before,
  # even when built with HDF5 enabled.
  find_package(
    HDF5
    COMPONENTS CXX
    REQUIRED)
  if(NOT HDF5_FOUND)
    message(STATUS "HDF5 lacks C++ support - disabling feature")
    set(RMG_HAS_HDF5 0)
  else()
    set(RMG_HAS_HDF5 1)
  endif()
else()
  message(STATUS "Geant4 lacks HDF5 support - disabling feature")
  set(RMG_HAS_HDF5 0)
endif()

if(Geant4_usolids_FOUND)
  message(STATUS "Geant4 compiled with VecGeom support - enabling feature")
  list(APPEND g4_components usolids)
  list(APPEND remage_components VecGeom)
else()
  message(STATUS "Geant4 lacks VecGeom support - disabling feature")
endif()

if(Geant4_multithreaded_FOUND)
  message(STATUS "Geant4 compiled with multithreading support - enabling feature")
  list(APPEND g4_components multithreaded)
  list(APPEND remage_components Multithreaded)
else()
  message(STATUS "Geant4 lacks multithreading support - disabling feature")
endif()

if(Geant4_gdml_FOUND)
  message(STATUS "Geant4 compiled with GDML support - enabling feature")
  set(RMG_HAS_GDML 1)
  list(APPEND g4_components gdml)
  list(APPEND remage_components GDML)
else()
  message(STATUS "Geant4 lacks GDML support - disabling feature")
  set(RMG_HAS_GDML 0)
endif()

# Define useful Geant4 functions and macros
include(${Geant4_USE_FILE})

option(RMG_USE_ROOT "Build remage with ROOT support" OFF)

# Find ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT ${RMG_ROOT_MINIMUM_VERSION} CONFIG QUIET COMPONENTS Core Tree)
if(ROOT_FOUND)
  message(STATUS "Found ROOT v" ${ROOT_VERSION} ", support enabled")
  list(APPEND remage_components ROOT)
else()
  if(RMG_USE_ROOT)
    find_package(ROOT ${RMG_ROOT_MINIMUM_VERSION} CONFIG REQUIRED COMPONENTS Core Tree)
  else()
    message(STATUS "ROOT not found, support disabled")
  endif()
endif()

option(RMG_USE_BXDECAY0 "Build remage with BxDecay0 support" OFF)

# find BxDecay0
find_package(BxDecay0 ${RMG_BXDECAY0_MINIMUM_VERSION} QUIET COMPONENTS Geant4)
set(BxDecay0_THREADSAFE 0)
if(BxDecay0_FOUND)
  message(STATUS "Found BxDecay0 v" ${BxDecay0_VERSION} ", support enabled")
  list(APPEND remage_components BxDecay0)
  if(BxDecay0_VERSION VERSION_GREATER "1.1.0")
    set(BxDecay0_THREADSAFE 1)
    message(STATUS "Found BxDecay0 v" ${BxDecay0_VERSION} ", BxDecay is thread-safe")
  endif()
else()
  if(RMG_USE_BXDECAY0)
    find_package(BxDecay0 ${RMG_BXDECAY0_MINIMUM_VERSION} REQUIRED COMPONENTS Geant4)
  else()
    message(STATUS "BxDecay0 not found, support disabled")
  endif()
endif()

# set minimum C++ standard
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "CMAKE_CXX_STANDARD is c++" ${CMAKE_CXX_STANDARD})

add_subdirectory(src)

option(RMG_BUILD_DOCS "Build remage documentation" OFF)
if(RMG_BUILD_DOCS)
  add_subdirectory(docs)
endif()

include(CTest)
add_subdirectory(tests)

# export targets for dependent projects
install(
  EXPORT remageTargets
  NAMESPACE RMG::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/remage)

# add support for find_package()
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/remageConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/remageConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/remage)

# create version file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/remageConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion)

# install
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/remageConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/remageConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/remage)

# write useful post-install setup scripts
include(Toolchain)
create_mage_toolchain()

include(DependencyGraph)
gen_dep_graph(pdf)

message(STATUS "")
message(STATUS "WWW: remage install prefix set to ${CMAKE_INSTALL_PREFIX}")
message(STATUS "WWW: if the install location is non-standard, don't forget to")
message(STATUS "WWW: update the relevant environment variables:")
message(STATUS "WWW:   export PATH=${CMAKE_INSTALL_PREFIX}/bin:$PATH")
message(STATUS "WWW:   export LD_LIBRARY_PATH=${CMAKE_INSTALL_PREFIX}/lib:$LD_LIBRARY_PATH")
message(
  STATUS "WWW:   export CMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}/lib/cmake:$CMAKE_PREFIX_PATH")
message(STATUS "")
