#!/bin/bash

set -euo pipefail

rmg="$1"
python_path="$2"
macro="macros/$3"

output_plain="${3/.mac/-plain.lh5}"
output_group="${3/.mac/-group.lh5}"

set -vx

# run the same simulation with the output at the file root and in the /sim group.
"$rmg" -g gdml/geometry.gdml -o "$output_plain" -w -- "$macro"
"$rmg" -g gdml/geometry.gdml -o "$output_group" -w --output-group sim -- "$macro"

# both files must have the same contents, just one level deeper.
"$python_path" ./verify_lh5_output_group.py sim "$output_plain" "$output_group"

set +vx
