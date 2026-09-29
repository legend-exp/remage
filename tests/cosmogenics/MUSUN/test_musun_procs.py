"""
The MUSUN generator must consume distinct blocks of the input file in multiprocessing mode,
i.e. the primary of (global) event i has to be taken from row i of the MUSUN file.
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
import pyg4ometry as pg4
import pytest
from lh5 import read_as
from remage import remage_run

MUSUN_FILE = "MUSUN_100_events.csv"
N_ROWS = 100

MACRO = [
    "/RMG/Output/ActivateOutputScheme Vertex",
    "/run/initialize",
    "/RMG/Generator/Confine UnConfined",
    "/RMG/Generator/Select MUSUNCosmicMuons",
    f"/RMG/Generator/MUSUNCosmicMuons/MUSUNFile {MUSUN_FILE}",
    "/run/beamOn {events}",
]


def _make_geometry(path: Path) -> None:
    reg = pg4.geant4.Registry()
    vacuum = pg4.geant4.MaterialPredefined("G4_Galactic", registry=reg)
    # the MUSUN vertices lie on the surface of a 16 x 16 x 20 m box.
    world_s = pg4.geant4.solid.Box("world", 30, 30, 30, registry=reg, lunit="m")
    world_l = pg4.geant4.LogicalVolume(world_s, vacuum, "world", registry=reg)
    reg.setWorld(world_l)
    w = pg4.gdml.Writer()
    w.addDetector(reg)
    w.write(str(path))


@pytest.mark.parametrize("procs", [1, 4])
def test_rows_match_event_ids(procs):
    tag = f"musun-procs-{procs}"
    for old in Path().glob(f"{tag}*"):
        old.unlink()

    gdml = Path(f"{tag}.gdml")
    _make_geometry(gdml)

    remage_run(
        [line.format(events=N_ROWS // procs) for line in MACRO],
        gdml_files=str(gdml),
        output=f"{tag}.lh5",
        procs=procs,
        merge_output_files=True,
        log_level="summary",
    )

    vtx = read_as("vtx", f"{tag}.lh5", "ak")
    order = np.argsort(vtx["evtid"].to_numpy(), kind="stable")
    evtid = vtx["evtid"].to_numpy()[order]
    pos_cm = np.stack(
        [vtx[c].to_numpy()[order] * 100 for c in ("xloc", "yloc", "zloc")], axis=1
    )

    rows = np.loadtxt(MUSUN_FILE)
    assert np.array_equal(evtid, np.arange(N_ROWS))
    assert np.allclose(pos_cm, rows[:, 3:6])
