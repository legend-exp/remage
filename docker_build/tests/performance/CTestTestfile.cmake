# CMake generated Testfile for 
# Source directory: /src/tests/performance
# Build directory: /src/docker_build/tests/performance
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[performance/combine-tracks]=] "/src/docker_build/python_venv/bin/python" "./combine_tracks_benchmark.py" "/src/docker_build/python_venv/bin/remage")
set_tests_properties([=[performance/combine-tracks]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/performance/CMakeLists.txt;12;add_test;/src/tests/performance/CMakeLists.txt;0;")
