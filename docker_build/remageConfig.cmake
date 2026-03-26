
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was remageConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

# custom code to check for remage components
# the 'remage_components' list is configure in the main CMakeLists.txt
set(_supported_components Multithreaded;GDML)

foreach(_comp ${remage_FIND_COMPONENTS})
  if (NOT _comp IN_LIST _supported_components)
    set(remage_FOUND False)
    set(remage_NOT_FOUND_MESSAGE "Unsupported component: ${_comp}. Recompile remage with the corresponding option.")
  endif()
endforeach()

# now look for remage dependencies as well, with the correct components remage was compiled with
include(CMakeFindDependencyMacro)

find_dependency(Geant4 11.2.2 REQUIRED COMPONENTS multithreaded;gdml)

if(0)
    find_dependency(ROOT 6.06 REQUIRED COMPONENTS Core Tree)
endif()

if(0)
    find_dependency(BxDecay0 1.0.10 REQUIRED COMPONENTS Geant4)
endif()

find_dependency(fmt REQUIRED)

# finally import targets.
include("${CMAKE_CURRENT_LIST_DIR}/remageTargets.cmake")
