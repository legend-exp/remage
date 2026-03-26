# CMake generated Testfile for 
# Source directory: /src/tests/particlefilter
# Build directory: /src/docker_build/tests/particlefilter
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[particlefilter/basicfilter]=] "/src/docker_build/python_venv/bin/python" "run-test-basicfilter.py")
set_tests_properties([=[particlefilter/basicfilter]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "extra;flaky" _BACKTRACE_TRIPLES "/src/tests/particlefilter/CMakeLists.txt;12;add_test;/src/tests/particlefilter/CMakeLists.txt;0;")
add_test([=[particlefilter/killvolume]=] "/src/docker_build/python_venv/bin/python" "run-test-killvolume.py")
set_tests_properties([=[particlefilter/killvolume]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/particlefilter/CMakeLists.txt;15;add_test;/src/tests/particlefilter/CMakeLists.txt;0;")
add_test([=[particlefilter/keepvolume]=] "/src/docker_build/python_venv/bin/python" "run-test-keepvolume.py")
set_tests_properties([=[particlefilter/keepvolume]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/particlefilter/CMakeLists.txt;18;add_test;/src/tests/particlefilter/CMakeLists.txt;0;")
add_test([=[particlefilter/killprocess]=] "/src/docker_build/python_venv/bin/python" "run-test-killprocess.py")
set_tests_properties([=[particlefilter/killprocess]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/particlefilter/CMakeLists.txt;21;add_test;/src/tests/particlefilter/CMakeLists.txt;0;")
add_test([=[particlefilter/keepprocess]=] "/src/docker_build/python_venv/bin/python" "run-test-keepprocess.py")
set_tests_properties([=[particlefilter/keepprocess]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/particlefilter/CMakeLists.txt;24;add_test;/src/tests/particlefilter/CMakeLists.txt;0;")
