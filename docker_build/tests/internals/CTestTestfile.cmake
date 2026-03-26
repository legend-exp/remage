# CMake generated Testfile for 
# Source directory: /src/tests/internals
# Build directory: /src/docker_build/tests/internals
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[internals/build-test-random]=] "/usr/bin/cmake" "--build" "/src/docker_build" "--config" "RelWithDebInfo" "--target" "test-random")
set_tests_properties([=[internals/build-test-random]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_SETUP "random-fixture" _BACKTRACE_TRIPLES "/src/tests/internals/CMakeLists.txt;4;add_test;/src/tests/internals/CMakeLists.txt;0;")
add_test([=[internals/random-Tasking]=] "/src/docker_build/tests/internals/test-random" "Tasking")
set_tests_properties([=[internals/random-Tasking]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "random-fixture" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/internals/CMakeLists.txt;11;add_test;/src/tests/internals/CMakeLists.txt;0;")
add_test([=[internals/random-MT]=] "/src/docker_build/tests/internals/test-random" "MT")
set_tests_properties([=[internals/random-MT]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "random-fixture" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/internals/CMakeLists.txt;11;add_test;/src/tests/internals/CMakeLists.txt;0;")
add_test([=[internals/random-Serial]=] "/src/docker_build/tests/internals/test-random" "Serial")
set_tests_properties([=[internals/random-Serial]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "random-fixture" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/internals/CMakeLists.txt;11;add_test;/src/tests/internals/CMakeLists.txt;0;")
