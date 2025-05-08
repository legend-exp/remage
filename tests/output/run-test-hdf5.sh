#!/bin/bash

set -euo pipefail

rmg="$1"
python_path="$2"
remage_to_lh5="$3"
macro="macros/$4"

output="${4/.mac/.hdf5}"
output_lh5="${4/.mac/.lh5}"
output_lh5_jag="${4/.mac/-jag.lh5}"
output_dump="${output}ls"
output_dump_lh5="${output}ls-lh5"
output_dump_lh5_jag="${output}jag-ls-lh5"
output_exp="macros/${4/.mac/.hdf5ls}"
output_exp_lh5="macros/${4/.mac/.hdf5ls-lh5}"
output_exp_lh5_jag="macros/${4/.mac/-jag.hdf5ls-lh5}"


# -----------------------
# TEST remage HDF5 output
# -----------------------

# run remage, produce hdf5 output.
"$rmg" -g gdml/geometry.gdml -o "$output" --flat-output -w -- "$macro"

# extract written hdf5 structure.
"$python_path" ./visit-hdf5.py "$output" > "$output_dump"

# compare structure with expectation.
# this verifies that we have
#   - hits in all detectors (otherwise some (`/pages` dsets would be missing)
#   - the correct det_uid/ntuple configuration
diff "$output_dump" "$output_exp"

# in-place convert the HDF file to LH5.
"$remage_to_lh5" "$output"


# ------------------------------
# TEST remage-to-lh5 CLI utility
# ------------------------------

# extract written lh5 structure & compare with expectation.
"$python_path" ./visit-hdf5.py "$output" --dump-attrs > "$output_dump_lh5"
diff "$output_dump_lh5" "$output_exp_lh5"


# -----------------------------
# TEST remage direct LH5 output
# -----------------------------

# run remage, produce lh5 output.
"$rmg" -g gdml/geometry.gdml -o "$output_lh5" --flat-output -w -- "$macro"

# extract written lh5 structure & compare with expectation.
"$python_path" ./visit-hdf5.py "$output_lh5" --dump-attrs > "$output_dump_lh5"
diff "$output_dump_lh5" "$output_exp_lh5"

# run remage, produce *jagaped* lh5 output.
"$rmg" -g gdml/geometry.gdml -o "$output_lh5_jag" -w -- "$macro"

# extract written lh5 structure & compare with expectation.
"$python_path" ./visit-hdf5.py "$output_lh5_jag" --dump-attrs > "$output_dump_lh5_jag"
diff "$output_dump_lh5_jag" "$output_exp_lh5_jag"
