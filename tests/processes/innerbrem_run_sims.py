from __future__ import annotations

import time

import dbetto
from reboost.build_hit import build_hit
from remage import remage_run


def run_reboost(f, reboost_config="config/hit_config.yaml"):
    stp_files = [f"{f}.stp.lh5"]
    hit_files = [f"{f}.lh5"]

    args = dbetto.AttrsDict({})
    _, _ = build_hit(
        reboost_config,
        args=args,
        stp_files=stp_files,
        glm_files=None,
        hit_files=hit_files,
        buffer=10_000_000,
        overwrite=True,
    )


t0 = time.time()
remage_run(
    macros="macros/run_IB.mac",
    gdml_files="gdml/geometry.gdml",
    output="output_IB.stp.lh5",
    overwrite_output=True,
)
tib = time.time() - t0
run_reboost("output_IB")

t0 = time.time()
remage_run(
    macros="macros/run_noIB.mac",
    gdml_files="gdml/geometry.gdml",
    output="output_noIB.stp.lh5",
    overwrite_output=True,
)
tno = time.time() - t0
print("simulation time ratio (IB/no IB)", tib / tno)
run_reboost("output_noIB")

remage_run(
    macros="macros/run_IB_track.mac",
    gdml_files="gdml/geometry.gdml",
    output="track.lh5",
    overwrite_output=True,
)
