# List here manually all source files. Using GLOB is bad, see:
# https://cmake.org/cmake/help/latest/command/file.html?highlight=Note#filesystem

set(_root ${CMAKE_SOURCE_DIR})

set(PROJECT_PUBLIC_HEADERS
    ${_root}/include/RMGNavigationTools.hh
    ${_root}/include/RMGVGenerator.hh
    ${_root}/include/RMGVertexConfinement.hh
    ${_root}/include/RMGGeneratorGPS.hh
    ${_root}/include/RMGVVertexGenerator.hh
    ${_root}/include/RMGMasterGenerator.hh
    ${_root}/include/RMGGeneratorCosmicMuons.hh
    ${_root}/include/RMGGeneratorUtil.hh
    ${_root}/include/RMGGeneratorG4Gun.hh
    ${_root}/include/RMGLog.hh
    ${_root}/include/RMGLog.icc
    ${_root}/include/ProjectInfo.hh
    ${_root}/include/RMGDetectorConstruction.hh
    ${_root}/include/RMGRunAction.hh
    ${_root}/include/RMGEventAction.hh
    ${_root}/include/RMGStackingAction.hh
    ${_root}/include/RMGSteppingAction.hh
    ${_root}/include/RMGTrackingAction.hh
    ${_root}/include/RMGUserAction.hh
    ${_root}/include/RMGManager.hh
    ${_root}/include/RMGRun.hh
    ${_root}/include/RMGMaterialTable.hh
    ${_root}/include/RMGPhysics.hh
    ${_root}/include/RMGTools.hh
    ${_root}/include/RMGTools.icc
)

set(PROJECT_SOURCES
    ${_root}/src/RMGNavigationTools.cc
    ${_root}/src/RMGGeneratorUtil.cc
    ${_root}/src/RMGMasterGenerator.cc
    ${_root}/src/RMGGeneratorCosmicMuons.cc
    ${_root}/src/RMGVertexConfinement.cc
    ${_root}/src/RMGLog.cc
    ${_root}/src/RMGDetectorConstruction.cc
    ${_root}/src/RMGEventAction.cc
    ${_root}/src/RMGRunAction.cc
    ${_root}/src/RMGStackingAction.cc
    ${_root}/src/RMGSteppingAction.cc
    ${_root}/src/RMGTrackingAction.cc
    ${_root}/src/RMGUserAction.cc
    ${_root}/src/RMGManager.cc
    ${_root}/src/RMGMaterialTable.cc
    ${_root}/src/RMGPhysics.cc
)

if(BxDecay0_FOUND)
    list(APPEND PROJECT_PUBLIC_HEADERS
        ../include/RMGGeneratorDecay0.hh
    )

    list(APPEND PROJECT_SOURCES
        RMGGeneratorDecay0.cc
    )
endif()

add_library(${CMAKE_PROJECT_NAME} SHARED ${PROJECT_PUBLIC_HEADERS} ${PROJECT_SOURCES})

# link against dependent libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    PUBLIC ${Geant4_LIBRARIES})

if(BxDecay0_FOUND)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PUBLIC
            BxDecay0::BxDecay0
            BxDecay0::BxDecay0_Geant4)
endif()

if(ROOT_FOUND)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PUBLIC
            ROOT::Core ROOT::Tree ROOT::Hist)
endif()

# Ensure clients are aware of the minimum C++ standard we were compiled with
target_compile_features(${CMAKE_PROJECT_NAME} PUBLIC cxx_std_${CMAKE_CXX_STANDARD})

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>"
        $<INSTALL_INTERFACE:include>)

# attach project version to shared library
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    INTERFACE_${CMAKE_PROJECT_NAME}_MAJOR_VERSION ${PROJECT_VERSION_MAJOR})

set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING ${CMAKE_PROJECT_NAME}_MAJOR_VERSION)

# write ProjectInfo.hh
# no need to install, it is included in the header list above
configure_file(${CMAKE_SOURCE_DIR}/cmake/ProjectInfo.hh.in
    ${CMAKE_SOURCE_DIR}/include/ProjectInfo.hh @ONLY)

# install CMake targets
install(TARGETS ${CMAKE_PROJECT_NAME}
    EXPORT ${CMAKE_PROJECT_NAME}Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install headers
install(DIRECTORY ../include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
