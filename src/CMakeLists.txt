# List here manually all source files. Using GLOB is bad, see:
# https://cmake.org/cmake/help/latest/command/file.html?highlight=Note#filesystem

set(PROJECT_PUBLIC_HEADERS
    ../include/RMGNavigationTools.hh

    ../include/RMGVGenerator.hh
    ../include/RMGGeneratorVolumeConfinement.hh
    ../include/RMGGeneratorVolumeConfinementMessenger.hh
    ../include/RMGGeneratorSPS.hh
    ../include/RMGVGeneratorPrimaryPosition.hh
    ../include/RMGGeneratorPrimary.hh
    ../include/RMGGeneratorUtil.hh
    ../include/RMGGeneratorPrimaryMessenger.hh
    ../include/RMGGeneratorG4Gun.hh

    ../include/RMGVOutputManager.hh
    ../include/RMGLog.hh
    ../include/RMGLog.icc
    ../include/ProjectInfo.hh

    ../include/RMGManagementDetectorConstruction.hh
    ../include/RMGManagementRunAction.hh
    ../include/RMGManagementEventAction.hh
    ../include/RMGManagementEventActionMessenger.hh
    ../include/RMGManagementStackingAction.hh
    ../include/RMGManagementSteppingAction.hh
    ../include/RMGManagementTrackingAction.hh
    ../include/RMGManagementUserAction.hh
    ../include/RMGManager.hh
    ../include/RMGRun.hh

    ../include/RMGMaterialTable.hh

    ../include/RMGUIcmdStepLimit.hh
    ../include/RMGProcessesList.hh
    ../include/RMGProcessesMessenger.hh

    ../include/RMGTools.hh
    ../include/RMGTools.icc
    ../include/RMGMessengerTools.icc
)

set(PROJECT_SOURCES
    geometry/RMGNavigationTools.cc

    generators/RMGGeneratorUtil.cc
    generators/RMGGeneratorPrimary.cc
    generators/RMGGeneratorPrimaryMessenger.cc
    generators/RMGGeneratorVolumeConfinement.cc
    generators/RMGGeneratorVolumeConfinementMessenger.cc

    io/RMGLog.cc
    # io/RMGVOutputManager.cc

    management/RMGManagementDetectorConstruction.cc
    management/RMGManagementEventAction.cc
    management/RMGManagementEventActionMessenger.cc
    management/RMGManagementRunAction.cc
    management/RMGManagementStackingAction.cc
    management/RMGManagementSteppingAction.cc
    management/RMGManagementTrackingAction.cc
    management/RMGManagementUserAction.cc
    management/RMGManager.cc

    materials/RMGMaterialTable.cc

    processes/RMGProcessesList.cc
    processes/RMGProcessesMessenger.cc
    processes/RMGUIcmdStepLimit.cc

    tools/RMGManagementTools.cc
    tools/RMGMessengerTools.cc
)

if(BxDecay0_FOUND)
    list(APPEND PROJECT_PUBLIC_HEADERS
        ../include/RMGGeneratorDecay0.hh
    )

    list(APPEND PROJECT_SOURCES
        generators/RMGGeneratorDecay0.cc
    )
endif()

add_library(${CMAKE_PROJECT_NAME} SHARED ${PROJECT_PUBLIC_HEADERS} ${PROJECT_SOURCES})

# link against dependent libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    PUBLIC
        ${Geant4_LIBRARIES})

if(BxDecay0_FOUND)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PUBLIC
            BxDecay0::BxDecay0
            BxDecay0::BxDecay0_Geant4)
endif()

if(ROOT_FOUND)
    target_link_libraries(${CMAKE_PROJECT_NAME}
        PUBLIC
            ROOT::Core)
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
