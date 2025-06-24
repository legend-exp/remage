# List here manually all source files. Using GLOB is bad, see:
# https://cmake.org/cmake/help/latest/command/file.html?highlight=Note#filesystem

set(_root ${PROJECT_SOURCE_DIR})

set(PROJECT_PUBLIC_HEADERS
    ${_root}/include/RMGAnalysisReader.hh
    ${_root}/include/RMGDetectorHit.hh
    ${_root}/include/RMGDetectorMetadata.hh
    ${_root}/include/RMGExceptionHandler.hh
    ${_root}/include/RMGEventAction.hh
    ${_root}/include/RMGGeneratorCosmicMuons.hh
    ${_root}/include/RMGGeneratorFromFile.hh
    ${_root}/include/RMGGeneratorMUSUNCosmicMuons.hh
    ${_root}/include/RMGGeneratorG4Gun.hh
    ${_root}/include/RMGGeneratorGPS.hh
    ${_root}/include/RMGGeneratorUtil.hh
    ${_root}/include/RMGGermaniumDetector.hh
    ${_root}/include/RMGGermaniumOutputScheme.hh
    ${_root}/include/RMGGrabmayrGCReader.hh
    ${_root}/include/RMGIsotopeFilterScheme.hh
    ${_root}/include/RMGIpc.hh
    ${_root}/include/RMGLog.hh
    ${_root}/include/RMGManager.hh
    ${_root}/include/RMGMasterGenerator.hh
    ${_root}/include/RMGNeutronCaptureProcess.hh
    ${_root}/include/RMGNavigationTools.hh
    ${_root}/include/RMGOpticalDetector.hh
    ${_root}/include/RMGOpticalOutputScheme.hh
    ${_root}/include/RMGOpWLSProcess.hh
    ${_root}/include/RMGParticleFilterScheme.hh
    ${_root}/include/RMGOutputManager.hh
    ${_root}/include/RMGOutputTools.hh
    ${_root}/include/RMGPhysics.hh
    ${_root}/include/RMGRunAction.hh
    ${_root}/include/RMGRun.hh
    ${_root}/include/RMGScintillatorDetector.hh
    ${_root}/include/RMGScintillatorOutputScheme.hh
    ${_root}/include/RMGStackingAction.hh
    ${_root}/include/RMGSteppingAction.hh
    ${_root}/include/RMGTools.hh
    ${_root}/include/RMGTrackingAction.hh
    ${_root}/include/RMGTrackOutputScheme.hh
    ${_root}/include/RMGUserAction.hh
    ${_root}/include/RMGUserInit.hh
    ${_root}/include/RMGVertexConfinement.hh
    ${_root}/include/RMGVertexFromFile.hh
    ${_root}/include/RMGVGenerator.hh
    ${_root}/include/RMGVOutputScheme.hh
    ${_root}/include/RMGVVertexGenerator.hh
    ${_root}/include/RMGVertexOutputScheme.hh
    ${_root}/include/RMGWorkerInitialization.hh)

set(PROJECT_SOURCES
    ${_root}/src/RMGAnalysisReader.cc
    ${_root}/src/RMGDetectorHit.cc
    ${_root}/src/RMGExceptionHandler.cc
    ${_root}/src/RMGHardware.cc
    ${_root}/src/RMGHardwareMessenger.cc
    ${_root}/src/RMGEventAction.cc
    ${_root}/src/RMGGeneratorCosmicMuons.cc
    ${_root}/src/RMGGeneratorFromFile.cc
    ${_root}/src/RMGGeneratorMUSUNCosmicMuons.cc
    ${_root}/src/RMGGeneratorUtil.cc
    ${_root}/src/RMGGermaniumDetector.cc
    ${_root}/src/RMGGermaniumOutputScheme.cc
    ${_root}/src/RMGGrabmayrGCReader.cc
    ${_root}/src/RMGIpc.cc
    ${_root}/src/RMGIsotopeFilterScheme.cc
    ${_root}/src/RMGLog.cc
    ${_root}/src/RMGManager.cc
    ${_root}/src/RMGMasterGenerator.cc
    ${_root}/src/RMGNavigationTools.cc
    ${_root}/src/RMGNeutronCaptureProcess.cc
    ${_root}/src/RMGOpticalDetector.cc
    ${_root}/src/RMGOpticalOutputScheme.cc
    ${_root}/src/RMGOpWLSProcess.cc
    ${_root}/src/RMGParticleFilterScheme.cc
    ${_root}/src/RMGOutputManager.cc
    ${_root}/src/RMGOutputTools.cc
    ${_root}/src/RMGPhysics.cc
    ${_root}/src/RMGRunAction.cc
    ${_root}/src/RMGScintillatorDetector.cc
    ${_root}/src/RMGScintillatorOutputScheme.cc
    ${_root}/src/RMGStackingAction.cc
    ${_root}/src/RMGSteppingAction.cc
    ${_root}/src/RMGTrackingAction.cc
    ${_root}/src/RMGTrackOutputScheme.cc
    ${_root}/src/RMGUserAction.cc
    ${_root}/src/RMGVertexConfinement.cc
    ${_root}/src/RMGVertexFromFile.cc
    ${_root}/src/RMGVertexOutputScheme.cc)

# Write RMGConfig.hh
# no need to install, it is included in the header list above
configure_file(${PROJECT_SOURCE_DIR}/cmake/RMGConfig.hh.in
               ${PROJECT_SOURCE_DIR}/include/RMGConfig.hh @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/cmake/RMGVersion.hh.in
               ${PROJECT_SOURCE_DIR}/include/RMGVersion.hh @ONLY)

if(BxDecay0_FOUND)
  list(APPEND PROJECT_PUBLIC_HEADERS ${_root}/include/RMGGeneratorDecay0.hh)

  list(APPEND PROJECT_SOURCES ${_root}/src/RMGGeneratorDecay0.cc)
endif()

if(RMG_HAS_HDF5)
  list(APPEND PROJECT_PUBLIC_HEADERS ${_root}/include/RMGConvertLH5.hh)

  list(APPEND PROJECT_SOURCES ${_root}/src/RMGConvertLH5.cc)
endif()

add_library(remage SHARED ${PROJECT_PUBLIC_HEADERS} ${PROJECT_SOURCES})

if(RMG_BUILD_EXAMPLES)
  # add an alias to support building examples both stand-alone as well as a part of the
  # main project.
  add_library(RMG::remage ALIAS remage)
endif()

# Ensure clients are aware of the minimum C++ standard we were compiled with
target_compile_features(remage PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

target_include_directories(remage PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                                         $<INSTALL_INTERFACE:include/remage>)

# link against dependent libraries
target_link_libraries(
  remage
  PUBLIC ${Geant4_LIBRARIES} fmt::fmt
  PRIVATE magic_enum::magic_enum)

if(BxDecay0_FOUND)
  target_link_libraries(remage PRIVATE BxDecay0::BxDecay0_Geant4)
endif()

if(ROOT_FOUND)
  target_link_libraries(remage PUBLIC ROOT::Core ROOT::Tree)
endif()

if(RMG_HAS_HDF5)
  target_include_directories(remage PRIVATE ${HDF5_INCLUDE_DIRS})
  target_link_libraries(remage PUBLIC ${HDF5_CXX_LIBRARIES})
endif()

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
add_executable(remage-cli-cpp ${_root}/src/remage.cc)
target_link_libraries(
  remage-cli-cpp
  PUBLIC remage
  PRIVATE CLI11::CLI11 magic_enum::magic_enum)
set_target_properties(remage-cli-cpp PROPERTIES OUTPUT_NAME remage-cpp)

# executable for dumping all docs
add_executable(remage-doc-dump-cli EXCLUDE_FROM_ALL ${_root}/src/remage-doc-dump.cc)
target_link_libraries(
  remage-doc-dump-cli
  PUBLIC remage
  PRIVATE CLI11::CLI11 magic_enum::magic_enum)
set_target_properties(remage-doc-dump-cli PROPERTIES OUTPUT_NAME remage-doc-dump)
if(BxDecay0_FOUND)
  target_link_libraries(remage-doc-dump-cli PRIVATE BxDecay0::BxDecay0_Geant4)
endif()
add_custom_target(
  remage-doc-dump
  COMMAND remage-doc-dump-cli --manual ${CMAKE_BINARY_DIR}/rmg-manual.txt
  COMMAND ${_root}/docs/g4manual2md.py ${CMAKE_BINARY_DIR}/rmg-manual.txt
  WORKING_DIRECTORY ${_root}/docs)

# executable for converting remage output HDF5 with ntuples to LH5 tables
if(RMG_HAS_HDF5)
  add_executable(remage-to-lh5 ${_root}/src/remage-to-lh5.cc)
  target_link_libraries(
    remage-to-lh5
    PUBLIC remage
    PRIVATE CLI11::CLI11 magic_enum::magic_enum)
  set_target_properties(remage-to-lh5 PROPERTIES OUTPUT_NAME remage-to-lh5)

  add_executable(remage-from-lh5 ${_root}/src/remage-from-lh5.cc)
  target_link_libraries(
    remage-from-lh5
    PUBLIC remage
    PRIVATE CLI11::CLI11 magic_enum::magic_enum)
  set_target_properties(remage-from-lh5 PROPERTIES OUTPUT_NAME remage-from-lh5)
endif()

# install CMake targets
install(
  TARGETS remage
  EXPORT remageTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# install headers
install(
  DIRECTORY ../include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/remage
  PATTERN EcoMug EXCLUDE)

# install CLI binaries
install(TARGETS remage-cli-cpp RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(RMG_HAS_HDF5)
  install(TARGETS remage-to-lh5 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
