# CMake generated Testfile for 
# Source directory: /src/tests/basics
# Build directory: /src/docker_build/tests/basics
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[basics/macro-substitution]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/geometry.gdml" "-o" "none" "--macro-substitutions" "ENERGY=1000" "GENERATOR=GPS" "--" "macros/run.mac")
set_tests_properties([=[basics/macro-substitution]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" _BACKTRACE_TRIPLES "/src/tests/basics/CMakeLists.txt;12;add_test;/src/tests/basics/CMakeLists.txt;0;")
add_test([=[basics/macro-substitution-missing]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/geometry.gdml" "-s" "ENERGY=1000" "-o" "none" "--" "macros/run.mac")
set_tests_properties([=[basics/macro-substitution-missing]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" WILL_FAIL "TRUE" _BACKTRACE_TRIPLES "/src/tests/basics/CMakeLists.txt;17;add_test;/src/tests/basics/CMakeLists.txt;0;")
add_test([=[basics/invalid-xml]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/geometry_invalid_xml.gdml" "-o" "none" "--macro-substitutions" "ENERGY=1000" "GENERATOR=GPS" "--" "macros/run.mac")
set_tests_properties([=[basics/invalid-xml]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" WILL_FAIL "TRUE" _BACKTRACE_TRIPLES "/src/tests/basics/CMakeLists.txt;22;add_test;/src/tests/basics/CMakeLists.txt;0;")
add_test([=[basics-mp/invalid-xml]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/geometry_invalid_xml.gdml" "-o" "none" "-P" "2" "--macro-substitutions" "ENERGY=1000" "GENERATOR=GPS" "--" "macros/run.mac")
set_tests_properties([=[basics-mp/invalid-xml]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "mp" PASS_REGULAR_EXPRESSION "exited with signal SIGTERM.*exited with signal SIGTERM" _BACKTRACE_TRIPLES "/src/tests/basics/CMakeLists.txt;28;add_test;/src/tests/basics/CMakeLists.txt;0;")
add_test([=[basics-mp/exit]=] "./run-mp-test-exit.sh" "/src/docker_build/python_venv/bin/remage")
set_tests_properties([=[basics-mp/exit]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" LABELS "mp;extra" PASS_REGULAR_EXPRESSION "SIGTERM.*SIGABRT|SIGABRT.*SIGTERM" _BACKTRACE_TRIPLES "/src/tests/basics/CMakeLists.txt;36;add_test;/src/tests/basics/CMakeLists.txt;0;")
add_test([=[basics/rand-seed]=] "/src/docker_build/python_venv/bin/remage" "-g" "gdml/geometry.gdml" "-o" "none" "--rand-seed" "123" "--macro-substitutions" "ENERGY=1000" "GENERATOR=GPS" "--" "macros/run.mac")
set_tests_properties([=[basics/rand-seed]=] PROPERTIES  ENVIRONMENT_MODIFICATION "MPLCONFIGDIR=set:/src/tests;RMG_STATS_FACTOR=set:1" PASS_REGULAR_EXPRESSION "CLHEP::HepRandom seed changed to: 123" _BACKTRACE_TRIPLES "/src/tests/basics/CMakeLists.txt;42;add_test;/src/tests/basics/CMakeLists.txt;0;")
