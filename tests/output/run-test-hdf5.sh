#!/bin/bash

set -euo pipefail

rmg="$1"
python_path="$2"
bindir="$(dirname "$python_path")/"
if [[ "$bindir" == "./" ]]; then
    bindir=""
fi
lh5ls="${bindir}lh5ls"
macro="macros/$3"

is_mt="${4-}"
extra_args=""
if [[ "$is_mt" == "mt" ]]; then
    extra_args="-m -t 2"
elif [[ "$is_mt" == "mp" ]]; then
    extra_args="-m -P 2"
fi

output_h5="${3/.mac/.hdf5}"
output_lh5="${3/.mac/.lh5}"
output_lh5_jag="${3/.mac/-pproc.lh5}"

if [[ "$is_mt" != "" ]]; then
    output_lh5="${output_lh5/.lh5/-$is_mt.lh5}"
    output_lh5_jag="${output_lh5_jag/.lh5/-$is_mt.lh5}"
fi

output_dump_h5="${output_h5}.ls"
output_dump_lh5="${output_lh5}.ls"
output_dump_lh5_jag="${output_lh5_jag}.ls"

output_exp_h5="dumps/${3/.mac/.hdf5.ls}"
output_exp_lh5="dumps/${3/.mac/.lh5.ls}"
output_exp_lh5_jag="dumps/${3/.mac/-pproc.lh5.ls}"

echo "$rmg"
"$python_path" -c "import lgdo; print(lgdo.__version__)"
"$lh5ls" --version h
"$python_path" -c "import os; print(os.environ)"

set -vx

# -----------------------
# TEST remage HDF5 output
# -----------------------

if [[ "$is_mt" == "" ]]; then
    # run remage, produce hdf5 output.
    "$rmg" -g gdml/geometry.gdml -o "$output_h5" --flat-output -w -- "$macro"

    # extract written hdf5 structure.
    "$python_path" ./visit-hdf5.py "$output_h5" > "$output_dump_h5"

    # compare structure with expectation.
    # this verifies that we have
    #   - hits in all detectors (otherwise some (`/pages` dsets would be missing)
    #   - the correct det_uid/ntuple configuration
    diff -u "$output_dump_h5" "$output_exp_h5"
fi


# -----------------------------
# TEST remage direct LH5 output
# -----------------------------

# run remage, produce lh5 output.
# shellcheck disable=SC2086
"$rmg" -g gdml/geometry.gdml -o "$output_lh5" --flat-output -w $extra_args -- "$macro"

# extract written lh5 structure & compare with expectation.
"$lh5ls" -a "$output_lh5" | sed -r 's/\x1B\[[0-9;]*[mK]//g' > "$output_dump_lh5"
diff -u "$output_dump_lh5" "$output_exp_lh5"

# -------------------------------------
# TEST remage post-precessed LH5 output
# -------------------------------------

# run remage, produce *jagged* lh5 output.
# shellcheck disable=SC2086
"$rmg" -g gdml/geometry.gdml -o "$output_lh5_jag" -w $extra_args -- "$macro"

# extract written lh5 structure & compare with expectation.
"$lh5ls" -a "$output_lh5_jag" | sed -r 's/\x1B\[[0-9;]*[mK]//g' > "$output_dump_lh5_jag"
diff -u "$output_dump_lh5_jag" "$output_exp_lh5_jag"

set +vx
