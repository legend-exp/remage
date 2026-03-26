# CMake generated Testfile for 
# Source directory: /src/tests/observables
# Build directory: /src/docker_build/tests/observables
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[observables-ge/gen-gdml]=] "/src/docker_build/python_venv/bin/python" "make_gdml.py")
set_tests_properties([=[observables-ge/gen-gdml]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_SETUP "observables-ge-gdml-fixture" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/observables/CMakeLists.txt;13;add_test;/src/tests/observables/CMakeLists.txt;0;")
add_test([=[observables-ge/vis]=] "/src/docker_build/python_venv/bin/remage" "macros/vis.mac" "-g" "gdml/geometry.gdml" "-o" "none" "-l" "detail")
set_tests_properties([=[observables-ge/vis]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "observables-ge-gdml-fixture" LABELS "extra;vis;val" _BACKTRACE_TRIPLES "/src/tests/observables/CMakeLists.txt;19;add_test;/src/tests/observables/CMakeLists.txt;0;")
add_test([=[observables-ge/run]=] "/src/docker_build/python_venv/bin/python" "./run_sim.py" "/src/docker_build/python_venv/bin/remage")
set_tests_properties([=[observables-ge/run]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "observables-ge-gdml-fixture" FIXTURES_SETUP "observables-ge-run-fixture" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/observables/CMakeLists.txt;26;add_test;/src/tests/observables/CMakeLists.txt;0;")
add_test([=[observables-ge/tracks-bulk-no-limit]=] "/src/docker_build/python_venv/bin/python" "./plot_steps.py" "out/beta_bulk/step_limits/max_None/stp/out.lh5" "tracks-bulk-no-limit")
set_tests_properties([=[observables-ge/tracks-bulk-no-limit]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "observables-ge-run-fixture" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/observables/CMakeLists.txt;32;add_test;/src/tests/observables/CMakeLists.txt;0;")
add_test([=[observables-ge/tracks-bulk-10um-limit]=] "/src/docker_build/python_venv/bin/python" "./plot_steps.py" "out/beta_bulk/step_limits/max_10/stp/out.lh5" "tracks-bulk-10um-limit")
set_tests_properties([=[observables-ge/tracks-bulk-10um-limit]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "observables-ge-run-fixture" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/observables/CMakeLists.txt;38;add_test;/src/tests/observables/CMakeLists.txt;0;")
add_test([=[observables-ge/observables]=] "/src/docker_build/python_venv/bin/python" "./plot_observables.py" "beta-observables")
set_tests_properties([=[observables-ge/observables]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "observables-ge-run-fixture" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/observables/CMakeLists.txt;44;add_test;/src/tests/observables/CMakeLists.txt;0;")
