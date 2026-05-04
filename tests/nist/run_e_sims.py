from __future__ import annotations

import os
from multiprocessing import Pool
from pathlib import Path

import numpy as np
from remage import remage_run

Path("output/").mkdir(parents=True, exist_ok=True)

n_proc = int(os.environ["RMG_STATS_FACTOR"])
n_events = 500 * n_proc

energies = np.geomspace(30, 50000, num=15).astype(int)  # in keV
step_sizes = (1, 0.1, 0.01, 0.001, 0.0001)  # in m
range_energies = np.array([300, 500, 1000, 1500, 2000, 3000, 4000])  # in keV
range_step_sizes = (1,)  # in m

sims = []

for mat in ("ge", "ar", "cu"):
    for step_size in step_sizes:  # in m
        for energy in energies:  # in keV
            sims.append(
                {
                    "macros": "macros/run.mac",
                    "macro_substitutions": {
                        "ENERGY": energy,
                        "EVENTS": n_events,
                        "MAX_STEP_SIZE_IN_M": step_size,
                        "FLUCT": "true",
                    },
                    "gdml_files": f"gdml/geometry-{mat}.gdml",
                    "output": f"output/electrons-{mat}-{energy}-keV-{step_size}-m.lh5",
                    "overwrite_output": True,
                    "flat_output": True,
                    "log_level": "summary",
                }
            )

for mat in ("ge", "ar", "cu"):
    for step_size in range_step_sizes:  # in m
        for energy in range_energies:  # in keV
            sims.append(
                {
                    "macros": "macros/run.mac",
                    "macro_substitutions": {
                        "ENERGY": energy,
                        "EVENTS": n_events,
                        "MAX_STEP_SIZE_IN_M": step_size,
                        "FLUCT": "true",
                    },
                    "gdml_files": f"gdml/geometry-{mat}.gdml",
                    "output": f"output/electrons-range-{mat}-{energy}-keV-{step_size}-m.lh5",
                    "overwrite_output": True,
                    "flat_output": True,
                    "log_level": "summary",
                }
            )


def _run(sim):
    remage_run(**sim)


if __name__ == "__main__":
    with Pool(n_proc) as pool:
        pool.map(_run, sims)
