#!/bin/bash

REMAGE_PYEXE="$1"

# prepare this test. We intentionally do not set the override file flag (-w) below.
# make sure that one process exits with ::fatal because it finds an output file.
rm test_mp_out*.lh5
touch test_mp_out_p1.lh5

# run enough events, so that the helper thread has enough time to observe the crash of the second process.
${REMAGE_PYEXE} -g gdml/geometry.gdml -o test_mp_out.lh5 -P 2 --macro-substitutions ENERGY=1000000 GENERATOR=GPS -- macros/run.mac
