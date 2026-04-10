from __future__ import annotations

import sys

import awkward as ak
import hist
from lgdo import lh5
from matplotlib import colors
from matplotlib import pyplot as plt
from reboost.shape.cluster import apply_cluster, cluster_by_step_length, step_lengths

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["font.size"] = 12
style = {"yerr": False, "flow": None, "fill": True, "lw": 0.6, "alpha": 0.5}

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["figure.figsize"] = (12, 4)
plt.rcParams["font.size"] = 12

# Get the BuPu colormap
cmap = plt.get_cmap("cividis")


def plot_tracks(_data, idx, savename=None):
    _fig, ax = plt.subplots(figsize=(6, 6))

    data = _data[_data.evtid == idx]

    data_tmp = ak.Array(
        {name: ak.flatten(data[name]) for name in ["xloc", "yloc", "zloc", "trackid"]}
    )

    x0, z0 = data_tmp[0].xloc, data_tmp[0].zloc

    for _id, track in enumerate(data_tmp.trackid):
        label = "tracks" if _id == 0 else None

        ax.plot(
            1000 * data_tmp[data_tmp.trackid == track].xloc - 1000 * x0,
            1000 * data_tmp[data_tmp.trackid == track].zloc - 1000 * z0,
            alpha=1,
            linewidth=2,
            label=label,
            color="tab:blue",
        )

    ax.scatter(
        1000 * data_tmp.xloc - 1000 * x0,
        1000 * data_tmp.zloc - 1000 * z0,
        s=20,
        label="steps",
        color="tab:red",
    )

    prefix = "m"
    ax.set_xlabel(f"x - x0 [{prefix}m]")
    ax.set_ylabel(f"z -z0 [{prefix}m]")

    ax.legend(fontsize=14)

    if savename is not None:
        plt.savefig(savename)


def plot_hist2d(
    dist, step_len, n=-1, high_stp=300, high_dist=5000, bins=500, savename=None
):
    _fig, ax = plt.subplots(1, 1, figsize=(12, 4))

    # Create a 2D histogram
    h = ax.hist2d(
        dist[:n].to_numpy(),
        step_len[:n].to_numpy(),
        bins=bins,
        range=[[0, high_dist], [0, high_stp]],
        cmap="BuPu",
        norm=colors.LogNorm(),
    )

    # Add colorbar
    plt.colorbar(h[3], ax=ax, label="Counts")
    ax.set_xlabel("Distance to Surface [um]")
    ax.set_ylabel("Step length [um]")

    if savename is not None:
        plt.savefig(savename)


def plot_steps(steps, bins=100, range=(0, 100), savename=None):
    _fig, ax = plt.subplots(figsize=(12, 4))
    h = hist.new.Reg(bins, range[0], range[1], name="steps").Double().fill(steps)
    h.plot(ax=ax, **style)
    ax.set_ylabel("Counts")
    ax.set_xlabel("Step size [um]")
    ax.set_xlim(range[0], range[1])
    ax.set_yscale("linear")

    if savename is not None:
        plt.savefig(savename)


path = sys.argv[1]
name = sys.argv[2]
shaped = lh5.read("stp/germanium", path).view_as("ak", with_units=True)

plot_tracks(shaped, 0, f"{name}.tracks.out0.png")
plot_tracks(shaped, 1, f"{name}.tracks.out1.png")
plot_tracks(shaped, 2, f"{name}.tracks.out2.png")


cluster_idx = cluster_by_step_length(
    shaped.trackid,
    shaped.xloc,
    shaped.yloc,
    shaped.zloc,
    shaped.dist_to_surf,
    surf_cut=0,
    threshold_in_mm=100,
    threshold_surf_in_mm=0,
)

cluster_x = apply_cluster(cluster_idx, shaped.xloc)
cluster_y = apply_cluster(cluster_idx, shaped.yloc)
cluster_z = apply_cluster(cluster_idx, shaped.zloc)

cluster_dist_to_surf = apply_cluster(cluster_idx, shaped.dist_to_surf)

step_len = ak.flatten(1e6 * ak.flatten(step_lengths(cluster_x, cluster_y, cluster_z)))

distance_to_surface = 1e6 * shaped.dist_to_surf
dist = 1e6 * ak.flatten(ak.flatten(cluster_dist_to_surf[:, :, :-1], axis=-2), axis=-1)

# plot the step hist and the step lengths
plot_hist2d(
    dist,
    step_len,
    -1,
    bins=100,
    high_dist=5000,
    high_stp=150,
    savename=f"{name}.step-vs-dist.png",
)
plot_hist2d(
    dist,
    step_len,
    -1,
    bins=100,
    high_dist=100,
    high_stp=100,
    savename=f"{name}.step-vs-dist-zoom.png",
)
plot_steps(step_len + 0.1, range=(0, 100), bins=500, savename=f"{name}.step-hist.png")


# plot activeness
