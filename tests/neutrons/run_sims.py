from __future__ import annotations

import os
from multiprocessing import Pool

from remage import remage_run

n_proc = int(os.environ.get("RMG_STATS_FACTOR", "1"))

DEFAULTS = {
    "overwrite_output": True,
}

CASES = {
    "evap": {
        "macros": "macros/general.mac",
        "gdml_files": "gdml/geometry.gdml",
        "output": "neutron_captures_evap.lh5",
        "macro_substitutions": {"EVAP": "true"},
    },
    "shielding": {
        "macros": "macros/general.mac",
        "gdml_files": "gdml/geometry.gdml",
        "output": "neutron_captures_shielding.lh5",
        "macro_substitutions": {"EVAP": "false"},
    },
    "normal": {
        "macros": "macros/gadolinium.mac",
        "gdml_files": "gdml/nist_gd_world.gdml",
        "output": "neutron_captures_normal.lh5",
    },
    "rmgncapture": {
        "macros": "macros/gadolinium_rmgncapture.mac",
        "gdml_files": "gdml/nist_gd_world.gdml",
        "output": "neutron_captures_rmgncapture.lh5",
    },
    "steel": {
        "macros": "macros/steel.mac",
        "gdml_files": "gdml/nist_steel_world.gdml",
        "output": "neutron_captures_steel.lh5",
    },
    "energy_scan": {
        "macros": "macros/energy_scan.mac",
        "gdml_files": "gdml/nist_gd_world.gdml",
        "output": "neutron_energy_scan.lh5",
    },
    "argon36": {
        "macros": "macros/argon_transport.mac",
        "gdml_files": "gdml/largon_36_world.gdml",
        "output": "neutron_argon36_transport.lh5",
        "flat_output": True,
    },
    "argon38": {
        "macros": "macros/argon_transport.mac",
        "gdml_files": "gdml/largon_38_world.gdml",
        "output": "neutron_argon38_transport.lh5",
        "flat_output": True,
    },
    "argon40": {
        "macros": "macros/argon_transport.mac",
        "gdml_files": "gdml/largon_40_world.gdml",
        "output": "neutron_argon40_transport.lh5",
        "flat_output": True,
    },
}


def run_case(case: str) -> None:
    try:
        config = CASES[case]
    except KeyError:
        msg = f"Unknown case: {case}"
        raise ValueError(msg) from None

    remage_run(**DEFAULTS, **config)


def main() -> None:
    with Pool(n_proc) as pool:
        pool.map(run_case, CASES)


if __name__ == "__main__":
    main()
