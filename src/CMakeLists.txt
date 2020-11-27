# TODO: list here manually all source files
add_library(${PROJECT_TARNAME} SHARED
    generators/MGGeneratorUtil.cc
    generators/MGGeneratorPrimary.cc
    generators/MGGeneratorPrimaryMessenger.cc
    io/MGLog.cc
    io/MGVOutputManager.cc
    management/MGManagementEventAction.cc
    management/MGManagementEventActionMessenger.cc
    management/MGManagementRunAction.cc
    management/MGManagementStackingAction.cc
    management/MGManagementSteppingAction.cc
    management/MGManagementTrackingAction.cc
    management/MGManager.cc
    management/MGManagerMessenger.cc
    processes/MGProcessesList.cc
    processes/MGProcessesMessenger.cc
    processes/MGUIcmdStepLimit.cc
)

# TODO: list here manually all project sub-directories
set(COMPONENTS
    generators
    io
    management
    processes
)

if(${PROJECT_VERSION} NOT EQUAL "unknown")
set_target_properties(${PROJECT_TARNAME} PROPERTIES
    VERSION ${PROJECT_VERSION})
endif()

if(${PROJECT_VERSION_MAJOR} NOT EQUAL "unknown")
set_target_properties(${PROJECT_TARNAME} PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR})
endif()

foreach(_sub ${COMPONENTS})
    list(APPEND _full_include_paths "${CMAKE_CURRENT_SOURCE_DIR}/${_sub}/include")
endforeach()

target_include_directories(${PROJECT_TARNAME}
    PUBLIC
        "$<BUILD_INTERFACE:${_full_include_paths}>"
        $<INSTALL_INTERFACE:include/${PROJECT_TARNAME}>
    PRIVATE processes)

install(TARGETS ${PROJECT_TARNAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}) # This is for Windows

foreach(_sub ${COMPONENTS})
    list(APPEND _include_paths "${_sub}/include")
endforeach()

install(DIRECTORY "${_include_paths}"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_TARNAME}
    FILES_MATCHING PATTERN "*.hh")
