# CMake generated Testfile for 
# Source directory: /src/tests/nist
# Build directory: /src/docker_build/tests/nist
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[nist/gen-output]=] "/src/docker_build/python_venv/bin/python" "run_e_sims.py")
set_tests_properties([=[nist/gen-output]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_SETUP "output-fixture" LABELS "extra;val;val-only" _BACKTRACE_TRIPLES "/src/tests/nist/CMakeLists.txt;11;add_test;/src/tests/nist/CMakeLists.txt;0;")
add_test([=[nist/e-range]=] "/src/docker_build/python_venv/bin/python" "./test_electron_interactions.py")
set_tests_properties([=[nist/e-range]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "output-fixture" LABELS "extra;val;val-only" _BACKTRACE_TRIPLES "/src/tests/nist/CMakeLists.txt;15;add_test;/src/tests/nist/CMakeLists.txt;0;")
