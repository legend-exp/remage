cmake_minimum_required(VERSION 3.12)
project(remage-tests)

# this can be used as the path to the remage executable in the tests
# in case the remage-cpp executable is needed, one can just use the
# "remage-cli-cpp" target name (see add_test() docs)
get_target_property(REMAGE_PYEXE remage-cli PYEXE_PATH)
get_target_property(PYTHONPATH python-virtualenv PYTHONPATH)

add_subdirectory(basics)
add_subdirectory(confinement)
add_subdirectory(detectors)
add_subdirectory(distances)
add_subdirectory(hades)
add_subdirectory(internals)
add_subdirectory(nist)
add_subdirectory(output)
add_subdirectory(particlefilter)
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
