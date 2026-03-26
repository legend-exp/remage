# CMake generated Testfile for 
# Source directory: /src/tests/gammacorr
# Build directory: /src/docker_build/tests/gammacorr
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[gammacorr/pytest-all]=] "/src/docker_build/python_venv/bin/python" "-m" "pytest" "-s" "-vvv" ".")
set_tests_properties([=[gammacorr/pytest-all]=] PROPERTIES  DISABLED "True" ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "mt;extra;val;val-only" _BACKTRACE_TRIPLES "/src/tests/gammacorr/CMakeLists.txt;12;add_test;/src/tests/gammacorr/CMakeLists.txt;0;")
