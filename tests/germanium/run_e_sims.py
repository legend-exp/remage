from __future__ import annotations

import numpy as np
from remage import remage_run

base_args = [
    "--macro-substitutions",
    "ENERGY={}",
    "--gdml-files",
    "gdml/geometry.gdml",
    "--output-file",
    "electrons-ge-{}-keV.lh5",
    "--overwrite",
    "--quiet",
    "macros/run.mac",
]
print("remage", " ".join(base_args))

energies = np.concatenate(
    [np.arange(100, 2000, step=100), np.arange(2000, 5000, step=500)]
)

for energy in energies:  # in keV
    args = [arg.format(energy) for arg in base_args]
    remage_run(args)
