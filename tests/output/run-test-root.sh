#!/bin/bash

set -euo pipefail

rmg="$1"
macro="macros/$2"
output="${2/.mac/.root}"

# run remage, produce root output.
"$rmg" -g gdml/geometry.gdml -o "$output" -w -- "$macro"

# check that output file is there.
test -f "$output"
