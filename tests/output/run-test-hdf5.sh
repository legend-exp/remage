#!/bin/bash

set -euo pipefail

rmg="$1"
visit_hdf5="$2"
macro="macros/$3"

output="${3/.mac/.hdf5}"
output_dump="${output}ls"
output_exp="macros/${3/.mac/.hdf5ls}"

# run remage, produce hdf5 output.
"$rmg" -g gdml/geometry.gdml -o "$output" -- "$macro"

# extract written hdf5 structure.
"$visit_hdf5" "$output" > "$output_dump"

# compare structure with expectation.
# this verifies that we have
#   - hits in all detectors (otherwise some (`/pages` dsets would be missing)
#   - the correct det_uid/ntuple configuration
diff "$output_dump" "$output_exp"
