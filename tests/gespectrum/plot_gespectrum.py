from __future__ import annotations

from datetime import datetime

import awkward as ak
import dbetto
import lh5
import matplotlib.pyplot as plt
import numpy as np
import reboost
import reboost.hpge.surface
import reboost.math.functions
import reboost.math.stats
from pygeomhpges import make_hpge

d1 = datetime.strptime("2013-11-01", "%Y-%m-%d")
d2 = datetime.strptime("2021-01-18", "%Y-%m-%d")
time = (d2 - d1).days
activity = 87.0e3 * 2 ** (-abs(time) / 365 / 1.9116)  # Bq

qc_info = np.loadtxt("data/hades-data-info.txt", dtype=int)
qc_survival = qc_info[1] / qc_info[0]  # ratio of QC accepted / total
run_time = 3600 * 17  # s, already includes DAQ dead-time
n_decays = activity * run_time * qc_survival

n_sim = lh5.read("/number_of_simulated_events", "hades-sim.lh5").value
factor = n_decays / n_sim

# load data histograms and its binning.
bins = np.load("data/variable_bins.npy")
widths = np.diff(bins)

h = np.load("data/hades-data-var.npy") / widths
ha = np.load("data/hades-data-all-var.npy") / widths

bkg_live_time = 190800  # s, background run0001
h_bkg = np.load("data/hades-data-bkg-var.npy") / widths * (run_time / bkg_live_time)


def gauss_smear(arr_true: ak.Array, arr_reso: ak.Array) -> ak.Array:
    """Smear values with expected resolution.

    Samples from gaussian and shifts negative values to a fixed, tiny positive
    value.
    """
    arr_smear = reboost.math.stats.gaussian_sample(
        arr_true,
        arr_reso,
    )

    # energy can't be negative as a result of smearing
    return ak.where((arr_smear <= 0) & (arr_true >= 0), np.finfo(float).tiny, arr_smear)


def DEFAULT_ENERGY_RES_FUNC(energy):
    return 2.5 * np.sqrt(energy / 2039)  # FWHM


# load and post-process simulation data.
sim = lh5.read_as("/stp/det001", "hades-sim.lh5", "ak", with_units=True)
det_origins = lh5.read("/detector_origins/B00000B", "hades-sim.lh5")
det_origins = np.array(
    [det_origins.xloc.value, det_origins.yloc.value, det_origins.zloc.value]
)

fccd = 1.3
meta = dbetto.AttrsDict(dbetto.utils.load_dict("dummy-metadata.yaml"))
hpge_pyobj = make_hpge(
    meta["detectors"]["germanium"]["diodes"]["B00000B"], registry=None
)
distance_to_nplus = reboost.hpge.surface.distance_to_surface(
    sim.xloc,
    sim.yloc,
    sim.zloc,
    hpge_pyobj,
    det_origins * 1000,
    surface_type="nplus",
)

activeness = reboost.math.functions.piecewise_linear_activeness(
    distance_to_nplus, fccd_in_mm=fccd, dlf=0.5
)
active_energy = ak.sum(sim.edep * activeness, axis=-1)
smeared_energy = gauss_smear(
    active_energy, DEFAULT_ENERGY_RES_FUNC(active_energy) / 2.35482
)


def plot_hist(add_before: bool = False):
    h_sim = np.histogram(smeared_energy, bins)[0]
    h_sim = h_sim / widths * factor + h_bkg

    fig, (ax0, ax1) = plt.subplots(
        2, 1, sharex=True, height_ratios=(1, 0.2), figsize=(10, 3), layout="constrained"
    )
    fig.get_layout_engine().set(hspace=0)
    ax0.tick_params(labelbottom=False)

    if add_before:
        ax0.stairs(
            ha,
            bins,
            fill=True,
            color="tab:blue",
            label="before QC scaled",
            alpha=0.2,
        )
    ax0.stairs(h, bins, fill=True, color="tab:blue", alpha=0.6, label="Data")

    ax0.stairs(h_sim, bins, label="MC", color="tab:red")

    ax0.set_yscale("log")
    ax0.set_xlim(500, 3000)
    ax0.set_ylim(1, 3e4)
    ax0.set_ylabel("counts / keV")
    ax0.legend()

    z_score = np.where(h > 0, (h_sim - h) * widths / np.sqrt(h * widths), np.nan)

    ax1.axhline(0, color="gray", zorder=0)
    for n, alpha in zip([3, 2, 1], [0.15, 0.3, 0.5], strict=True):
        ax1.axhspan(-n, n, color="tab:green", alpha=alpha, zorder=1)
    ax1.scatter((bins[:-1] + bins[1:]) / 2, z_score, color="black", s=2, zorder=100)
    ax1.set_ylim(-5, 5)
    ax1.set_xlabel("energy [keV]")
    ax1.set_ylabel(r"(MC$-$Data)/$\sigma$")

    return ax0, ax1, fig


ax0, ax1, fig = plot_hist()
ax0.set_xlim(10, 3500)
fig.savefig("hades-spectrum-full.output.png")

ax0.set_yscale("linear")
ax0.set_ylim(0, 250)
fig.savefig("hades-spectrum-linear.output.png")
