# Cmake function to find and use info from *-config scripts
# The function exports the following variables:
#
# - INCLUDE_DIRS_FROM_CONFIG
# - LIBS_DIRS_FROM_CONFIG

function(find_config_script NAME)
  find_program(${NAME}_CONFIG_EXECUTABLE ${NAME}-config)
  if(${NAME}_CONFIG_EXECUTABLE)
    # find header paths
    execute_process(
      COMMAND ${${NAME}_CONFIG_EXECUTABLE} --cflags
      OUTPUT_VARIABLE CFLAGS
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    separate_arguments(CFLAGS)
    string(REGEX MATCHALL "-I[^;]+" INCLUDE_DIRS "${CFLAGS}")
    string(REPLACE "-I" " " INCLUDE_DIRS "${INCLUDE_DIRS}")
    string(STRIP "${INCLUDE_DIRS}" INCLUDE_DIRS)
    separate_arguments(INCLUDE_DIRS)

    # find lib paths
    execute_process(
      COMMAND ${${NAME}_CONFIG_EXECUTABLE} --libs
      OUTPUT_VARIABLE LIBS
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    separate_arguments(LIBS)
    string(REGEX MATCHALL "-L[^;]+" LIBS_DIRS "${LIBS}")
    string(REPLACE "-L" " " LIBS_DIRS "${LIBS_DIRS}")
    string(STRIP "${LIBS_DIRS}" LIBS_DIRS)
    separate_arguments(LIBS_DIRS)

    set(INCLUDE_DIRS_FROM_CONFIG
        ${INCLUDE_DIRS}
        PARENT_SCOPE)
    set(LIBS_DIRS_FROM_CONFIG
        ${LIBS_DIRS}
        PARENT_SCOPE)

    mark_as_advanced(${NAME}_CONFIG_EXECUTABLE INCLUDE_DIRS_FROM_CONFIG LIBS_DIRS_FROM_CONFIG)
  else()
    message(WARNING "${NAME}-config executable not found")
  endif()
endfunction()
