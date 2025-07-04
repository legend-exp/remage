#!/bin/bash

set -euo pipefail

rmg="$1"
python_path="$2"
lh5ls="$(dirname "$python_path")/lh5ls"
macro="macros/$3"

output_h5="${3/.mac/.hdf5}"
output_lh5="${3/.mac/.lh5}"
output_lh5_jag="${3/.mac/-pproc.lh5}"

output_dump_h5="${output_h5}.ls"
output_dump_lh5="${output_lh5}.ls"
output_dump_lh5_jag="${output_lh5_jag}.ls"

output_exp_h5="dumps/${3/.mac/.hdf5.ls}"
output_exp_lh5="dumps/${3/.mac/.lh5.ls}"
output_exp_lh5_jag="dumps/${3/.mac/-pproc.lh5.ls}"

set -vx

# -----------------------
# TEST remage HDF5 output
# -----------------------

# run remage, produce hdf5 output.
"$rmg" -g gdml/geometry.gdml -o "$output_h5" --flat-output -w -- "$macro"

# extract written hdf5 structure.
"$python_path" ./visit-hdf5.py "$output_h5" > "$output_dump_h5"

# compare structure with expectation.
# this verifies that we have
#   - hits in all detectors (otherwise some (`/pages` dsets would be missing)
#   - the correct det_uid/ntuple configuration
diff "$output_dump_h5" "$output_exp_h5"


# -----------------------------
# TEST remage direct LH5 output
# -----------------------------

# run remage, produce lh5 output.
"$rmg" -g gdml/geometry.gdml -o "$output_lh5" --flat-output -w -- "$macro"

# extract written lh5 structure & compare with expectation.
"$lh5ls" -a "$output_lh5" | sed -r 's/\x1B\[[0-9;]*[mK]//g' > "$output_dump_lh5"
diff "$output_dump_lh5" "$output_exp_lh5"

# -------------------------------------
# TEST remage post-precessed LH5 output
# -------------------------------------

# run remage, produce *jagged* lh5 output.
"$rmg" -g gdml/geometry.gdml -o "$output_lh5_jag" -w -- "$macro"

# extract written lh5 structure & compare with expectation.
"$lh5ls" -a "$output_lh5_jag" | sed -r 's/\x1B\[[0-9;]*[mK]//g' > "$output_dump_lh5_jag"
diff "$output_dump_lh5_jag" "$output_exp_lh5_jag"

set +vx
