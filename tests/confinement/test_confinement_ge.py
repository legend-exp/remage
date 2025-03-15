from __future__ import annotations

import copy

import awkward as ak
import legendhpges as hpges
import numpy as np
import pyg4ometry as pg4
from lgdo import lh5
from matplotlib import pyplot as plt
from pygeomtools import get_sensvol_metadata
from scipy import stats

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["font.size"] = 12

gdml = "gdml/ge-array.gdml"
outfile = "test-confine.lh5"

# get the geometry
reg = pg4.gdml.Reader(gdml).getRegistry()
reg_tmp = pg4.geant4.Registry()
detectors = list(reg.physicalVolumeDict.keys())

detectors = [det for det in detectors if det[0] in ["V", "P", "B", "C"]]

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

# append with the uint and the local positions
vertices = lh5.read_as("stp/vertices", outfile, "ak")

uint = np.array(np.full_like(vertices.time, -1), dtype=int)
xlocal = np.array(1000 * vertices.xloc)
ylocal = np.array(1000 * vertices.yloc)
zlocal = np.array(1000 * vertices.zloc)

positions = np.array(
    np.transpose(
        np.vstack([vertices.xloc * 1000, vertices.yloc * 1000, vertices.zloc * 1000])
    )
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

vertices["uid"] = uint
vertices["xlocal"] = xlocal
vertices["ylocal"] = ylocal
vertices["zlocal"] = zlocal

steps = lh5.read_as("stp/germanium", outfile, "ak")
steps = ak.unflatten(steps, ak.run_lengths(steps.evtid))
uid = ak.fill_none(ak.firsts(steps.det_uid, axis=-1), -1)
evtid = ak.fill_none(ak.firsts(steps.evtid, axis=-1), -1)
hits = ak.Array({"evtid": evtid, "uid": uid})


def make_plot(vert, hit):
    fraction_vert = []
    fraction_vert_err = []
    fraction_hit = []
    expected_fraction = []
    names = []
    bad_uid = []

    n_tot = len(vert)

    vol_tot = 0
    N = 0

    for det in det_map:
        vol_tot += float(det_map[det]["hpge"].volume.magnitude)

        n_sel_vert = len(vert[vert.uid == det_map[det]["uint"]])
        n_sel_hit = len(hit[hit.uid == det_map[det]["uint"]])

        names.append(det)
        bad_uid.append(det_map[det]["uint"])

        fraction_vert.append(100 * n_sel_vert / n_tot)
        fraction_vert_err.append(100 * np.sqrt(n_sel_vert) / n_tot)
        fraction_hit.append(100 * n_sel_hit / n_tot)
        expected_fraction.append(float(det_map[det]["hpge"].volume.magnitude))
        N += 1

    expected_fraction = 100 * np.array(expected_fraction) / vol_tot
    fraction_vert = np.array(fraction_vert)
    fraction_vert_err = np.array(fraction_vert_err)

    # calculate the p-value
    # technically the correct proobability distribution is a multinomial since Ntot is fixed
    # I think that a product of Poisson will work fine since the overall poisson term is probably negligible,
    # just results in N - 1 not N degrees of freedom. Wilkes theorem should surely hold..

    test_stat = 2 * np.sum(
        n_tot * expected_fraction / 100
        - (fraction_vert * n_tot / 100)
        * (1 - np.log(fraction_vert / expected_fraction))
    )

    # should follow a chi2 distribution with N -1 dof

    p = stats.chi2.sf(test_stat, N - 1)
    sigma = stats.norm.ppf(1 - p)

    fig, ax = plt.subplots(2, 1, figsize=(12, 8), sharex=True)

    ax[0].errorbar(
        np.arange(len(names)),
        fraction_vert,
        yerr=fraction_vert_err,
        fmt=".",
        label="Vertices",
    )
    ax[0].errorbar(np.arange(len(names)) - 0.1, fraction_hit, fmt="x", label="Hits")

    ax[0].errorbar(np.arange(len(names)), expected_fraction, fmt=".", label="Expected")
    ax[0].set_ylabel("Fraction of vertices [%]")
    ax[0].set_xticks(np.arange(len(names)), names, rotation=90, fontsize=10)
    ax[0].legend()
    ax[0].grid()

    # residual
    ax[1].errorbar(
        np.arange(len(names)),
        100 * (fraction_vert - expected_fraction) / expected_fraction,
        yerr=100 * fraction_vert_err / expected_fraction,
        fmt=".",
        label="Vertices",
    )
    ax[1].errorbar(
        np.arange(len(names)) - 0.1,
        100 * (fraction_hit - expected_fraction) / expected_fraction,
        fmt="x",
        label="Hits",
    )
    ax[1].set_xlabel("Channel Index")
    ax[1].set_ylabel("Relative difference [%]")
    ax[1].set_xticks(np.arange(len(names)), names, rotation=90, fontsize=10)
    ax[1].axhline(y=0, color="red")
    ax[1].grid()
    fig.suptitle(f"confinement check for HPGes, p = {100 * p:.1e} %")
    caption = "The fraction of the vertices found inside each HPGe. This is compared to the expectation which is that the number "
    caption += "should be proportional to the volume of the HPGe. The top panel shows the fraction in each detector "
    caption += r"while the lower panel shows the relative difference in % from the expectation."
    plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
    plt.tight_layout(rect=[0, 0.12, 1, 1])

    return p, sigma


p, sigma = make_plot(vertices, hits)
plt.savefig("relative-ge.output.pdf")

assert sigma < 5
