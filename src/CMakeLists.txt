# List here manually all source files. Using GLOB is bad, see:
# https://cmake.org/cmake/help/latest/command/file.html?highlight=Note#filesystem

set(PROJECT_PUBLIC_HEADERS
    geometry/include/RMGNavigationTools.hh

    generators/include/RMGVGenerator.hh
    generators/include/RMGGeneratorVolumeConfinement.hh
    generators/include/RMGGeneratorVolumeConfinementMessenger.hh
    generators/include/RMGGeneratorSPS.hh
    generators/include/RMGVGeneratorPrimaryPosition.hh
    generators/include/RMGGeneratorPrimary.hh
    generators/include/RMGGeneratorUtil.hh
    generators/include/RMGGeneratorPrimaryMessenger.hh
    generators/include/RMGGeneratorG4Gun.hh

    io/include/RMGVOutputManager.hh
    io/include/RMGLog.hh
    io/include/RMGLog.icc
    io/include/ProjectInfo.hh

    management/include/RMGManagementDetectorConstruction.hh
    management/include/RMGManagementRunAction.hh
    management/include/RMGManagementEventAction.hh
    management/include/RMGManagementEventActionMessenger.hh
    management/include/RMGManagementStackingAction.hh
    management/include/RMGManagementSteppingAction.hh
    management/include/RMGManagementTrackingAction.hh
    management/include/RMGManagementUserAction.hh
    management/include/RMGManager.hh
    management/include/RMGManagerMessenger.hh
    management/include/RMGRun.hh

    materials/include/RMGMaterialTable.hh

    processes/include/RMGUIcmdStepLimit.hh
    processes/include/RMGProcessesList.hh
    processes/include/RMGProcessesMessenger.hh

    tools/include/RMGTools.hh
    tools/include/RMGMessengerTools.icc
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
    management/RMGManagerMessenger.cc

    materials/RMGMaterialTable.cc

    processes/RMGProcessesList.cc
    processes/RMGProcessesMessenger.cc
    processes/RMGUIcmdStepLimit.cc

    tools/RMGManagementTools.cc
    tools/RMGMessengerTools.cc
)

if(BxDecay0_FOUND)
    list(APPEND PROJECT_PUBLIC_HEADERS
        generators/include/RMGGeneratorDecay0.hh
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

# let the project know where to find its headers
set(COMPONENTS
    geometry
    generators
    io
    management
    materials
    processes
    tools
)

foreach(_sub ${COMPONENTS})
    list(APPEND _full_include_paths "${CMAKE_CURRENT_SOURCE_DIR}/${_sub}/include/")
endforeach()

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${_full_include_paths}>"
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
    ${CMAKE_SOURCE_DIR}/src/io/include/ProjectInfo.hh @ONLY)

# install CMake targets
install(TARGETS ${CMAKE_PROJECT_NAME}
    EXPORT ${CMAKE_PROJECT_NAME}Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install headers
install(FILES ${PROJECT_PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
