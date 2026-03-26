#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "RMG::remage" for configuration "RelWithDebInfo"
set_property(TARGET RMG::remage APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(RMG::remage PROPERTIES
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/libremage.so.0.1"
  IMPORTED_SONAME_RELWITHDEBINFO "libremage.so.0"
  )

list(APPEND _cmake_import_check_targets RMG::remage )
list(APPEND _cmake_import_check_files_for_RMG::remage "${_IMPORT_PREFIX}/lib/libremage.so.0.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
