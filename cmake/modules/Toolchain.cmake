# configure useful post-install scripts

macro(create_mage_toolchain)
  # create and eventually install configuration scripts
  configure_file(${PROJECT_SOURCE_DIR}/cmake/project-config.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config @ONLY)

  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config
            DESTINATION ${CMAKE_INSTALL_BINDIR})
endmacro()
