# CMake generated Testfile for 
# Source directory: /src/tests/processes
# Build directory: /src/docker_build/tests/processes
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[processes/innerbrem-run]=] "/src/docker_build/python_venv/bin/python" "./innerbrem_run_sims.py" "/src/docker_build/python_venv/bin/remage")
set_tests_properties([=[processes/innerbrem-run]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_SETUP "innerbrem-output-fixture" LABELS "extra;val;val-only" _BACKTRACE_TRIPLES "/src/tests/processes/CMakeLists.txt;12;add_test;/src/tests/processes/CMakeLists.txt;0;")
add_test([=[processes/innerbrem-plot]=] "/src/docker_build/python_venv/bin/python" "innerbrem_plot_effect.py")
set_tests_properties([=[processes/innerbrem-plot]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "innerbrem-output-fixture" LABELS "extra;val;val-only" _BACKTRACE_TRIPLES "/src/tests/processes/CMakeLists.txt;17;add_test;/src/tests/processes/CMakeLists.txt;0;")
add_test([=[processes/dump-physics]=] "/src/docker_build/python_venv/bin/python" "./dump_physics.py" "/src/docker_build/python_venv/bin/remage")
set_tests_properties([=[processes/dump-physics]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "val" _BACKTRACE_TRIPLES "/src/tests/processes/CMakeLists.txt;22;add_test;/src/tests/processes/CMakeLists.txt;0;")
