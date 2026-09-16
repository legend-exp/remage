from __future__ import annotations

import time

import awkward as ak
import lh5
import reboost
from lgdo import Array
from remage import remage_run


def run_pproc(f):
    """Sum the energy deposited in each hit of the germanium detector."""
    stps = lh5.read("stp/det001", f"{f}.stp.lh5")
    steps = stps.view_as("ak", with_units=True)
    edep = reboost.units.units_conv_ak(steps.edep, "keV")

    # carries over the evtid and t0 fields, i.e. what a TCM needs
    hits = reboost.init_hit_table(stps)
    hits.add_field(
        "active_energy",
        Array(ak.to_numpy(ak.sum(edep, axis=-1)), attrs={"units": "keV"}),
    )

    lh5.write(hits, "hit/det001", f"{f}.lh5", wo_mode="overwrite_file")


t0 = time.time()
remage_run(
    macros="macros/run_IB.mac",
    gdml_files="gdml/geometry.gdml",
    output="output_IB.stp.lh5",
    overwrite_output=True,
)
tib = time.time() - t0
run_pproc("output_IB")

t0 = time.time()
remage_run(
    macros="macros/run_noIB.mac",
    gdml_files="gdml/geometry.gdml",
    output="output_noIB.stp.lh5",
    overwrite_output=True,
)
tno = time.time() - t0
print("simulation time ratio (IB/no IB)", tib / tno)
run_pproc("output_noIB")

remage_run(
    macros="macros/run_IB_track.mac",
    gdml_files="gdml/geometry.gdml",
    output="track.lh5",
    overwrite_output=True,
)
