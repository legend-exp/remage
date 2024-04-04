# List here manually all source files. Using GLOB is bad, see:
# https://cmake.org/cmake/help/latest/command/file.html?highlight=Note#filesystem

set(_root ${PROJECT_SOURCE_DIR})

set(PROJECT_PUBLIC_HEADERS
    ${_root}/include/RMGEventAction.hh
    ${_root}/include/RMGGeneratorCosmicMuons.hh
    ${_root}/include/RMGGeneratorG4Gun.hh
    ${_root}/include/RMGGeneratorGPS.hh
    ${_root}/include/RMGGeneratorUtil.hh
    ${_root}/include/RMGGermaniumDetector.hh
    ${_root}/include/RMGGermaniumOutputScheme.hh
    ${_root}/include/RMGLog.hh
    ${_root}/include/RMGManager.hh
    ${_root}/include/RMGMasterGenerator.hh
    ${_root}/include/RMGNavigationTools.hh
    ${_root}/include/RMGOpticalDetector.hh
    ${_root}/include/RMGOpticalOutputScheme.hh
    ${_root}/include/RMGPhysics.hh
    ${_root}/include/RMGRunAction.hh
    ${_root}/include/RMGRun.hh
    ${_root}/include/RMGStackingAction.hh
    ${_root}/include/RMGSteppingAction.hh
    ${_root}/include/RMGTools.hh
    ${_root}/include/RMGTrackingAction.hh
    ${_root}/include/RMGUserAction.hh
    ${_root}/include/RMGVertexConfinement.hh
    ${_root}/include/RMGVertexFromFile.hh
    ${_root}/include/RMGVGenerator.hh
    ${_root}/include/RMGVOutputScheme.hh
    ${_root}/include/RMGVVertexGenerator.hh)

set(PROJECT_SOURCES
    ${_root}/src/RMGHardware.cc
    ${_root}/src/RMGHardwareMessenger.cc
    ${_root}/src/RMGEventAction.cc
    ${_root}/src/RMGGeneratorCosmicMuons.cc
    ${_root}/src/RMGGeneratorUtil.cc
    ${_root}/src/RMGGermaniumDetector.cc
    ${_root}/src/RMGGermaniumOutputScheme.cc
    ${_root}/src/RMGLog.cc
    ${_root}/src/RMGManager.cc
    ${_root}/src/RMGMasterGenerator.cc
    ${_root}/src/RMGNavigationTools.cc
    ${_root}/src/RMGOpticalDetector.cc
    ${_root}/src/RMGOpticalOutputScheme.cc
    ${_root}/src/RMGPhysics.cc
    ${_root}/src/RMGRunAction.cc
    ${_root}/src/RMGStackingAction.cc
    ${_root}/src/RMGSteppingAction.cc
    ${_root}/src/RMGTrackingAction.cc
    ${_root}/src/RMGUserAction.cc
    ${_root}/src/RMGVertexConfinement.cc
    ${_root}/src/RMGVertexFromFile.cc)

# Write RMGConfig.hh
# no need to install, it is included in the header list above
configure_file(${PROJECT_SOURCE_DIR}/cmake/RMGConfig.hh.in
               ${PROJECT_SOURCE_DIR}/include/RMGConfig.hh @ONLY)

if(BxDecay0_FOUND)
  list(APPEND PROJECT_PUBLIC_HEADERS ${_root}/include/RMGGeneratorDecay0.hh)

  list(APPEND PROJECT_SOURCES ${_root}/src/RMGGeneratorDecay0.cc)
endif()

add_library(remage SHARED ${PROJECT_PUBLIC_HEADERS} ${PROJECT_SOURCES})

# Ensure clients are aware of the minimum C++ standard we were compiled with
target_compile_features(remage PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

target_include_directories(remage PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
                                         $<INSTALL_INTERFACE:include>)

# Must use FMT_HEADER_ONLY absolutely everywhere or will get undefined symbols at link time
# TODO: Use imported target so we can make this transparent and also allow use of external fmt
target_compile_definitions(remage PUBLIC FMT_HEADER_ONLY)

# link against dependent libraries
target_link_libraries(remage PUBLIC ${Geant4_LIBRARIES})

if(BxDecay0_FOUND)
  target_link_libraries(remage PRIVATE BxDecay0::BxDecay0_Geant4)
endif()

if(ROOT_FOUND)
  target_link_libraries(remage PUBLIC ROOT::Core ROOT::Tree)
endif()

# Ensure clients are aware of the minimum C++ standard we were compiled with
target_compile_features(remage PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

target_include_directories(remage PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
                                         $<INSTALL_INTERFACE:include>)

# attach project version to shared library
set_target_properties(
  remage
  PROPERTIES VERSION ${PROJECT_VERSION}
             SOVERSION ${PROJECT_VERSION_MAJOR}
             INTERFACE_remage_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})

set_property(
  TARGET remage
  APPEND
  PROPERTY COMPATIBLE_INTERFACE_STRING remage_MAJOR_VERSION)

# executable for CLI
add_executable(remage-cli ${_root}/src/remage.cc)
target_link_libraries(remage-cli PUBLIC remage)
set_target_properties(remage-cli PROPERTIES OUTPUT_NAME remage)

# install CMake targets
install(
  TARGETS remage
  EXPORT remageTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# install headers
install(
  DIRECTORY ../include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PATTERN "CLI11" EXCLUDE
  PATTERN "EcoMug" EXCLUDE)

# install CLI binary
install(
  TARGETS remage-cli
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
