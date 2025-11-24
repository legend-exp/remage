from __future__ import annotations

import numpy as np
from remage import remage_run_from_args

base_args = [
    "--macro-substitutions",
    "ENERGY={}",
    "MAX_STEP_SIZE_IN_M=1",
    "--gdml-files",
    "gdml/geometry.gdml",
    "--output-file",
    "electrons-ge-{}-keV.lh5",
    "--flat-output",
    "--overwrite",
    "macros/run.mac",
]

energies = np.concatenate(
    [
        np.arange(30, 100, step=20),
        np.arange(100, 500, step=200),
        np.arange(500, 5000, step=500),
    ]
)

for energy in energies:  # in keV
    args = [arg.format(energy) for arg in base_args]
    print("remage", " ".join(args))
    remage_run_from_args(args, raise_on_error=True)
