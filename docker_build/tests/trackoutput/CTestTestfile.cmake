# CMake generated Testfile for 
# Source directory: /src/tests/trackoutput
# Build directory: /src/docker_build/tests/trackoutput
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[trackoutput/gen-output]=] "/src/docker_build/python_venv/bin/python" "run_sims_trackoutput.py")
set_tests_properties([=[trackoutput/gen-output]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_SETUP "track-output-fixture" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/trackoutput/CMakeLists.txt;12;add_test;/src/tests/trackoutput/CMakeLists.txt;0;")
add_test([=[trackoutput/test-output]=] "/src/docker_build/python_venv/bin/python" "test_trackoutput.py")
set_tests_properties([=[trackoutput/test-output]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "track-output-fixture" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/trackoutput/CMakeLists.txt;16;add_test;/src/tests/trackoutput/CMakeLists.txt;0;")
