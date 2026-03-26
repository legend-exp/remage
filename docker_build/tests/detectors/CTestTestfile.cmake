# CMake generated Testfile for 
# Source directory: /src/tests/detectors
# Build directory: /src/docker_build/tests/detectors
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[detectors/gen-output]=] "/src/docker_build/python_venv/bin/python" "./run_sims_det.py" "/src/docker_build/python_venv/bin/remage")
set_tests_properties([=[detectors/gen-output]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_SETUP "det-output-fixture" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/detectors/CMakeLists.txt;12;add_test;/src/tests/detectors/CMakeLists.txt;0;")
add_test([=[detectors/test-output]=] "/src/docker_build/python_venv/bin/python" "test_output.py")
set_tests_properties([=[detectors/test-output]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "det-output-fixture" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/detectors/CMakeLists.txt;16;add_test;/src/tests/detectors/CMakeLists.txt;0;")
