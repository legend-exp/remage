# List here manually all source files. Using GLOB is bad, see:
# https://cmake.org/cmake/help/latest/command/file.html?highlight=Note#filesystem

set(PROJECT_SOURCES
    geometry/RMGNavigationTools.cc

    generators/RMGGeneratorUtil.cc
    generators/RMGGeneratorPrimary.cc
    generators/RMGGeneratorPrimaryMessenger.cc
    generators/RMGGeneratorVolumeConfinement.cc
    generators/RMGGeneratorVolumeConfinementMessenger.cc

    io/RMGLog.cc
    io/RMGVOutputManager.cc

    management/RMGManagementEventAction.cc
    management/RMGManagementEventActionMessenger.cc
    management/RMGManagementRunAction.cc
    management/RMGManagementStackingAction.cc
    management/RMGManagementSteppingAction.cc
    management/RMGManagementTrackingAction.cc
    management/RMGManager.cc
    management/RMGManagerDetectorConstruction.cc
    management/RMGManagerMessenger.cc

    materials/RMGMaterialTable.cc
    materials/RMGMaterialTableMessenger.cc

    processes/RMGProcessesList.cc
    processes/RMGProcessesMessenger.cc
    processes/RMGUIcmdStepLimit.cc

    tools/RMGMessengerTools.cc
    tools/RMGManagementTools.cc
)

if(BxDecay0_FOUND)
    list(APPEND PROJECT_SOURCES
        generators/RMGGeneratorDecay0.cc
        generators/RMGGeneratorDecay0Messenger.cc
    )
endif()

add_library(${PROJECT_TARNAME} SHARED ${PROJECT_SOURCES})

# link against dependent libraries
target_link_libraries(${PROJECT_TARNAME}
    PUBLIC
        ${Geant4_LIBRARIES})

if(BxDecay0_FOUND)
target_link_libraries(${PROJECT_TARNAME}
    PUBLIC
        BxDecay0::BxDecay0)
        BxDecay0::BxDecay0_Geant4)
endif()

if(ROOT_FOUND)
target_link_libraries(${PROJECT_TARNAME}
    PUBLIC
        ROOT::Core)
endif()

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
    list(APPEND _full_include_paths "${CMAKE_CURRENT_SOURCE_DIR}/${_sub}/include")
endforeach()

target_include_directories(${PROJECT_TARNAME}
    PUBLIC
        "$<BUILD_INTERFACE:${_full_include_paths}>"
        $<INSTALL_INTERFACE:include/${PROJECT_TARNAME}>
    PRIVATE
        ${COMPONENTS})

# attach project version to shared library
if(${PROJECT_VERSION} NOT EQUAL "unknown")
set_target_properties(${PROJECT_TARNAME} PROPERTIES
    VERSION ${PROJECT_VERSION})
endif()

if(${PROJECT_VERSION_MAJOR} NOT EQUAL "unknown")
set_target_properties(${PROJECT_TARNAME} PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

# configure install
install(TARGETS ${PROJECT_TARNAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}) # This is for Windows

foreach(_sub ${COMPONENTS})
    list(APPEND _include_paths "${_sub}/include")
endforeach()

install(DIRECTORY "${_include_paths}"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_TARNAME}
    FILES_MATCHING PATTERN "*.hh")
