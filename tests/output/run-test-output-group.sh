#!/bin/bash

set -euo pipefail

rmg="$1"
python_path="$2"
macro="macros/$3"

output_plain="${3/.mac/-plain.lh5}"
output_group="${3/.mac/-group.lh5}"

set -vx

# run the same simulation with the output at the file root and in the /sim1 group.
"$rmg" -g gdml/geometry.gdml -o "$output_plain" -w -- "$macro"
"$rmg" -g gdml/geometry.gdml -o "$output_group" -w --output-group sim1 -- "$macro"

# the group must hold the same contents as the whole plain file.
"$python_path" ./verify_lh5_output_group.py sim1 "$output_plain" "$output_group"

# add a second simulation to the same file, in another group.
"$rmg" -g gdml/geometry.gdml -o "$output_group" -a --output-group sim2 -- "$macro"

# the second group must be complete and the first one must still be there.
"$python_path" ./verify_lh5_output_group.py sim2 "$output_plain" "$output_group"
"$python_path" ./verify_lh5_output_group.py sim1 "$output_plain" "$output_group"

set +vx
