# CMake generated Testfile for 
# Source directory: /src/tests/distances
# Build directory: /src/docker_build/tests/distances
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[distances-ge/gen-gdml]=] "/src/docker_build/python_venv/bin/python" "make_ge_gdml.py")
set_tests_properties([=[distances-ge/gen-gdml]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_SETUP "distance-gdml-fixture" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/distances/CMakeLists.txt;14;add_test;/src/tests/distances/CMakeLists.txt;0;")
add_test([=[distances-ge/gen-output-prestep]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/ge-array.gdml" "-w" "-o" "test-distance-prestep.lh5" "--flat-output" "--" "macros/test-ge-distance-prestep.mac")
set_tests_properties([=[distances-ge/gen-output-prestep]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "distance-gdml-fixture" FIXTURES_SETUP "distance-output-fixture-prestep" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/distances/CMakeLists.txt;20;add_test;/src/tests/distances/CMakeLists.txt;0;")
add_test([=[distances-ge/distance-prestep]=] "/src/docker_build/python_venv/bin/python" "./test_ge_distance.py" "test-distance-prestep.lh5" "prestep")
set_tests_properties([=[distances-ge/distance-prestep]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "distance-output-fixture-prestep" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/distances/CMakeLists.txt;28;add_test;/src/tests/distances/CMakeLists.txt;0;")
add_test([=[distances-ge/gen-output-poststep]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/ge-array.gdml" "-w" "-o" "test-distance-poststep.lh5" "--flat-output" "--" "macros/test-ge-distance-poststep.mac")
set_tests_properties([=[distances-ge/gen-output-poststep]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "distance-gdml-fixture" FIXTURES_SETUP "distance-output-fixture-poststep" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/distances/CMakeLists.txt;20;add_test;/src/tests/distances/CMakeLists.txt;0;")
add_test([=[distances-ge/distance-poststep]=] "/src/docker_build/python_venv/bin/python" "./test_ge_distance.py" "test-distance-poststep.lh5" "poststep")
set_tests_properties([=[distances-ge/distance-poststep]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "distance-output-fixture-poststep" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/distances/CMakeLists.txt;28;add_test;/src/tests/distances/CMakeLists.txt;0;")
add_test([=[distances-ge/gen-output-average]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/ge-array.gdml" "-w" "-o" "test-distance-average.lh5" "--flat-output" "--" "macros/test-ge-distance-average.mac")
set_tests_properties([=[distances-ge/gen-output-average]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "distance-gdml-fixture" FIXTURES_SETUP "distance-output-fixture-average" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/distances/CMakeLists.txt;20;add_test;/src/tests/distances/CMakeLists.txt;0;")
add_test([=[distances-ge/distance-average]=] "/src/docker_build/python_venv/bin/python" "./test_ge_distance.py" "test-distance-average.lh5" "average")
set_tests_properties([=[distances-ge/distance-average]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" FIXTURES_REQUIRED "distance-output-fixture-average" LABELS "extra;val" _BACKTRACE_TRIPLES "/src/tests/distances/CMakeLists.txt;28;add_test;/src/tests/distances/CMakeLists.txt;0;")
