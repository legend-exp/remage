cmake_minimum_required(VERSION 3.28)
project(remage-tests)

# ---------------------------------------------------------------------
# this can be used as the path to the remage executable in the tests
# in case the remage-cpp executable is needed, one can just use the
# "remage-cli-cpp" target name (see add_test() docs)
get_target_property(REMAGE_PYEXE remage-cli PYEXE_PATH)
get_target_property(PYTHONPATH python-virtualenv PYTHONPATH)

# ---------------------------------------------------------------------
# calculate a statistics boost factor depending on the machine.
if(RMG_VALIDATION_STATS_FACTOR)
  set(STATS_FACTOR ${RMG_VALIDATION_STATS_FACTOR})
else()
  include(ProcessorCount)
  ProcessorCount(_num_cores)

  math(EXPR STATS_FACTOR "${_num_cores} / 8")
  if(STATS_FACTOR LESS 1)
    set(STATS_FACTOR 1)
  endif()
endif()
message(STATUS "stats factor for validation tests: ${STATS_FACTOR}")

# ---------------------------------------------------------------------
# add tests from the sub-directories.

add_subdirectory(basics)
add_subdirectory(confinement)
add_subdirectory(detectors)
add_subdirectory(distances)
add_subdirectory(gammacorr)
add_subdirectory(hades)
add_subdirectory(internals)
add_subdirectory(nist)
add_subdirectory(output)
add_subdirectory(particlefilter)
add_subdirectory(performance)
add_subdirectory(processes)
add_subdirectory(python)
add_subdirectory(trackoutput)
add_subdirectory(observables)
add_subdirectory(vertex)

if(RMG_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(BxDecay0_FOUND)
  add_subdirectory(bxdecay0)
endif()

# ---------------------------------------------------------------------
# set global environment variables for all defined tests.
get_property(
  _subdirs
  DIRECTORY
  PROPERTY SUBDIRECTORIES)
foreach(_subdir ${_subdirs})
  get_property(
    _tests
    DIRECTORY ${_subdir}
    PROPERTY TESTS)

  foreach(_test ${_tests})
    # set the environment variables without overriding the ENVIRONMENT property.
    set_tests_properties(
      ${_test} DIRECTORY ${_subdir}
      PROPERTIES ENVIRONMENT_MODIFICATION
                 "MPLCONFIGDIR=set:${CMAKE_SOURCE_DIR}/tests;RMG_STATS_FACTOR=set:${STATS_FACTOR}")
  endforeach()
endforeach()
