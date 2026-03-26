# CMake generated Testfile for 
# Source directory: /src/tests/hades
# Build directory: /src/docker_build/tests/hades
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[hades/print-volumes.mac]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/main.gdml" "--" "macros/print-volumes.mac")
set_tests_properties([=[hades/print-volumes.mac]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" _BACKTRACE_TRIPLES "/src/tests/hades/CMakeLists.txt;20;add_test;/src/tests/hades/CMakeLists.txt;0;")
add_test([=[hades/run-2nbb.mac]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/main.gdml" "--" "macros/run-2nbb.mac")
set_tests_properties([=[hades/run-2nbb.mac]=] PROPERTIES  DISABLED "True" ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "extra" _BACKTRACE_TRIPLES "/src/tests/hades/CMakeLists.txt;20;add_test;/src/tests/hades/CMakeLists.txt;0;")
add_test([=[hades/vis-co60.mac]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/main.gdml" "--" "macros/vis-co60.mac")
set_tests_properties([=[hades/vis-co60.mac]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "vis" _BACKTRACE_TRIPLES "/src/tests/hades/CMakeLists.txt;20;add_test;/src/tests/hades/CMakeLists.txt;0;")
add_test([=[hades/vis-2nbb.mac]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/main.gdml" "--" "macros/vis-2nbb.mac")
set_tests_properties([=[hades/vis-2nbb.mac]=] PROPERTIES  DISABLED "True" ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "vis" _BACKTRACE_TRIPLES "/src/tests/hades/CMakeLists.txt;20;add_test;/src/tests/hades/CMakeLists.txt;0;")
add_test([=[hades-mt/print-volumes.mac]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/main.gdml" "-t" "4" "macros/print-volumes.mac")
set_tests_properties([=[hades-mt/print-volumes.mac]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "mt" PROCESSORS "4" _BACKTRACE_TRIPLES "/src/tests/hades/CMakeLists.txt;24;add_test;/src/tests/hades/CMakeLists.txt;0;")
add_test([=[hades/overlaps.mac]=] "/src/docker_build/python_venv/bin/remage" "--" "macros/overlaps.mac")
set_tests_properties([=[hades/overlaps.mac]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" PASS_REGULAR_EXPRESSION "GeomVol1002.*GeomVol1002" _BACKTRACE_TRIPLES "/src/tests/hades/CMakeLists.txt;51;add_test;/src/tests/hades/CMakeLists.txt;0;")
