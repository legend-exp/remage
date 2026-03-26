# CMake generated Testfile for 
# Source directory: /src/tests/python
# Build directory: /src/docker_build/tests/python
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[python/cli]=] "/src/docker_build/python_venv/bin/remage" "-q" "--help")
set_tests_properties([=[python/cli]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" _BACKTRACE_TRIPLES "/src/tests/python/CMakeLists.txt;11;add_test;/src/tests/python/CMakeLists.txt;0;")
add_test([=[python/pytest-wrapper]=] "/src/docker_build/python_venv/bin/python" "-m" "pytest" "-vvv")
set_tests_properties([=[python/pytest-wrapper]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "mt;extra" _BACKTRACE_TRIPLES "/src/tests/python/CMakeLists.txt;14;add_test;/src/tests/python/CMakeLists.txt;0;")
