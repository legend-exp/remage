# configure useful post-install scripts

macro(create_mage_toolchain)
    # create and eventually install configuration scripts
    configure_file(${CMAKE_SOURCE_DIR}/cmake/project-config.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config @ONLY)

    # don't install project-config on windows
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config
            DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()

    execute_process(COMMAND ${mgdo_CONFIG_EXECUTABLE} --prefix
        OUTPUT_VARIABLE MGDODIR)
    set(MAGEDIR ${CMAKE_SOURCE_DIR})

    find_program(Geant4_CONFIG_EXECUTABLE geant4-config)
    execute_process(COMMAND ${Geant4_CONFIG_EXECUTABLE} --prefix
        OUTPUT_VARIABLE Geant4_INSTALL_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${Geant4_CONFIG_EXECUTABLE} --version
        OUTPUT_VARIABLE Geant4_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    find_program(ROOT_CONFIG_EXECUTABLE root-config)
    execute_process(COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
        OUTPUT_VARIABLE ROOTSYS
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endmacro()
