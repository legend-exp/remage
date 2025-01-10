# test_ge_distances.py
# This test checks if the Geant4 calculated distances agree
# with the distances calculated by the HPGe class.
# Fails if the distances are not within a tolerance (default: 1 nm).

from __future__ import annotations

import copy

import awkward as ak
import legendhpges as hpges
import numpy as np
import pyg4ometry as pg4
from lgdo import lh5
from matplotlib import pyplot as plt
from pygeomtools import get_sensvol_metadata

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["font.size"] = 12

gdml = "gdml/ge-array.gdml"
outfile = "test-distance.lh5"

# get the geometry
reg = pg4.gdml.Reader(gdml).getRegistry()
reg_tmp = pg4.geant4.Registry()
detectors = list(reg.physicalVolumeDict.keys())

detectors = [det for det in detectors if det[0] in ["V", "P", "B", "C"]]
# Map uid to hpge
det_map = {
    det: {
        "uint": int(det[1:]),
        "pos": reg.physicalVolumeDict[det].position.eval(),
        "hpge": hpges.make_hpge(
            get_sensvol_metadata(reg, det), name=det, registry=reg_tmp
        ),
    }
    for idx, det in enumerate(detectors)
}

steps = lh5.read_as("stp/germanium", outfile, "ak")
# Transform the coordinates to the local detector frame
uint = np.array(np.full_like(steps.time, -1), dtype=int)
xlocal = np.array(1000 * steps.xloc)
ylocal = np.array(1000 * steps.yloc)
zlocal = np.array(1000 * steps.zloc)

positions = np.array(
    np.transpose(np.vstack([steps.xloc * 1000, steps.yloc * 1000, steps.zloc * 1000]))
)
for det in det_map:
    local_positions = copy.copy(positions)
    local_positions -= det_map[det]["pos"]

    is_inside = np.full(len(uint), False)
    is_inside[uint == -1] = det_map[det]["hpge"].is_inside(local_positions[uint == -1])

    uint[is_inside] = det_map[det]["uint"]
    xlocal[is_inside] -= det_map[det]["pos"][0]
    ylocal[is_inside] -= det_map[det]["pos"][1]
    zlocal[is_inside] -= det_map[det]["pos"][2]


steps["xlocal"] = xlocal
steps["ylocal"] = ylocal
steps["zlocal"] = zlocal
steps = ak.unflatten(steps, ak.run_lengths(steps.evtid))
uid = ak.fill_none(ak.firsts(steps.det_uid, axis=-1), -1)
dist_G4 = ak.fill_none(ak.firsts(steps.dist_to_surf * 1000, axis=-1), -1)
xlocal = ak.fill_none(ak.firsts(steps.xlocal, axis=-1), -1)
ylocal = ak.fill_none(ak.firsts(steps.ylocal, axis=-1), -1)
zlocal = ak.fill_none(ak.firsts(steps.zlocal, axis=-1), -1)
hits = ak.Array(
    {"dist_G4": dist_G4, "uid": uid, "xloc": xlocal, "yloc": ylocal, "zloc": zlocal}
)


def make_plot(hit, tolerance=1e-6):
    good_distance = True
    for idx, det in enumerate(det_map):
        temp = det_map[det]["uint"]
        sel_hit = hit[hit.uid == temp]

        coords = np.column_stack((sel_hit.xloc, sel_hit.yloc, sel_hit.zloc))
        dist_py = det_map[det]["hpge"].distance_to_surface(coords)
        # Check if all distances are within tolerance
        if np.sum(np.abs(sel_hit.dist_G4 - dist_py) > tolerance):
            good_distance = False

        # Now draw the plots
        hpges.draw.plot_profile(det_map[det]["hpge"], axes=axs[idx])

        rpos_loc = np.sqrt(sel_hit.xloc**2 + sel_hit.yloc**2)
        rng = np.random.default_rng()
        r = rng.choice([-1, 1], p=[0.5, 0.5], size=len(rpos_loc)) * rpos_loc
        z = sel_hit.zloc
        c = sel_hit.dist_G4
        cut = c < 5

        s = axs[idx].scatter(
            r[cut],
            z[cut],
            c=c[cut],
            marker=".",
            label="gen. points",
            cmap="BuPu",
        )

        if idx == 0:
            axs[idx].set_ylabel("Height [mm]")
        c = plt.colorbar(s)
        c.set_label("Distance [mm]")

        axs[idx].set_xlabel("Radius [mm]")

    return good_distance


num_dets = len(det_map)
cols = 2
rows = (num_dets + (cols - 1)) // cols

fig, axs = plt.subplots(rows, cols, figsize=(5 * cols, 5 * rows))
axs = axs.flatten()


are_distances_good = make_plot(hits)
plt.suptitle("Distance check for HPGes")
caption = "The distance to the surface of the HPGe detectors, as calculated by Geant4, "
caption += "illustrated within the HPGe outline drawn with legendhpges.\n"
caption += (
    "The test will fail if the difference between the distances calculated by Geant4 "
)
caption += "compared to the distances calculated by legendhpges is not within a tolerance of (default) 1 nm."
plt.figtext(0.02, 0.04, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])
# plt.tight_layout()
plt.savefig("distance-ge.output.pdf")

assert are_distances_good
