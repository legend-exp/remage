from __future__ import annotations

import copy
import json
import sys
from pathlib import Path

import awkward as ak
import hist
import lh5
import numpy as np
from matplotlib import pyplot as plt
from matplotlib.lines import Line2D
from mpl_toolkits.axes_grid1.inset_locator import inset_axes
from scipy.stats import beta, norm, poisson

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["figure.figsize"] = (12, 4)
plt.rcParams["font.size"] = 14

style = {
    "yerr": False,
    "flow": None,
    "fill": False,
    "lw": 1,
}

# Get the BuPu colormap
cmap = plt.get_cmap("cividis")
height = 40  # mm
radius = 40  # mm


def get_cylinder_dist(r, z, radius, height):
    a = np.array((height / 2.0 - z).to_numpy())
    b = np.array((z + height / 2).to_numpy())
    c = np.array((radius - r).to_numpy())

    return np.minimum(np.minimum(a, b), c)


# geometry of the plots meant for publications: they should fit on half an A4
# page without being scaled down further (which would shrink the fonts), and a
# roughly square aspect ratio wastes less space there than a wide one
PAPER_FIGSIZE = (5, 4.6)
# font sizes of the annotations inside those figures, relative to the axis
# labels (rcParams["font.size"])
SMALL_FONTSIZE = 11
TITLE_FONTSIZE = 13


def savefig(save_name):
    """Save the current figure, both as PNG and as PDF."""
    plt.savefig(save_name)
    plt.savefig(Path(save_name).with_suffix(".pdf"))


def get_lh5(generator, name, val, dist_low=None, dist_high=None):
    path = f"{generator}/{name}/max_{val}/"
    hit_directory = Path(f"out/{path}/hit/")

    data = lh5.read_as(
        "hit/germanium", f"{hit_directory}/out.lh5", "ak", with_units=True
    )
    verts = lh5.read_as("hit/vtx", f"{hit_directory}/out.lh5", "ak", with_units=True)
    verts["dist_to_surf"] = get_cylinder_dist(
        1000 * verts.rloc, 1000 * verts.zloc, radius, height
    )
    verts["dist_to_surf"] = ak.with_parameter(verts["dist_to_surf"], "units", "mm")

    if dist_low is not None:
        n_sel = ak.sum(
            (verts["dist_to_surf"] > dist_low) & (verts["dist_to_surf"] < dist_high)
        )
    else:
        n_sel = len(verts)

    hit_ids = np.searchsorted(verts.evtid, data.evtid)
    verts = verts[hit_ids]

    data["vert_rloc"] = verts.rloc
    data["vert_zloc"] = verts.zloc

    data["vert_dist_to_surf"] = get_cylinder_dist(
        1000 * data.vert_rloc, 1000 * data.vert_zloc, radius, height
    )
    data["vert_dist_to_surf"] = ak.with_parameter(
        data["vert_dist_to_surf"], "units", "mm"
    )

    if dist_low is not None:
        data = data[
            (data["vert_dist_to_surf"] > dist_low)
            & (data["vert_dist_to_surf"] < dist_high)
        ]

    return data, n_sel


def get_timing(generator, name, val):
    """Read the timing information stored by ``run_sim.py`` for one simulation."""
    path = Path(f"{generator}/{name}/max_{val}/")

    with (Path("out") / path / "timing.json").open(encoding="utf-8") as f:
        return json.load(f)


# line styles of the horizontal reference lines in plot_timing(). They have to be
# distinguishable from each other, since both are drawn in the same axes.
CPU_DEFAULT_LINESTYLE = (0, (5, 3))
SIZE_DEFAULT_LINESTYLE = (0, (5, 1.5, 1, 1.5))


# color used for everything that refers to the reference simulation, i.e. to the
# point of the scan the other ones are compared against
DEFAULT_COLOR = "tab:red"
# color used to point out where the remage default of the scanned setting lies.
# Distinct from DEFAULT_COLOR: the reference simulation and the default value of
# the setting are only the same thing if `default_x` is used.
MARK_COLOR = "dimgrey"


def mark_default_x(ax, default_x, label, color=DEFAULT_COLOR):
    ax.axvline(default_x, color=color, linestyle="--", zorder=0)
    ax.annotate(
        label,
        xy=(default_x, 0.02),
        xycoords=("data", "axes fraction"),
        xytext=(4, 0),
        textcoords="offset points",
        ha="left",
        va="bottom",
        fontsize=SMALL_FONTSIZE,
        color=color,
    )


def plot_timing(
    generators,
    name,
    values,
    unit,
    save_name,
    xlabel=None,
    default_label="Geant4 default",
    default_x=None,
    mark_x=None,
    mark_label="remage default",
    color_by_metric=False,
    title=None,
):
    """Plot CPU/wall-clock time and output size per simulated event vs. the setting.

    Both are shown relative to the reference simulation.

    With ``color_by_metric`` the metrics get a color each instead of the line
    style carrying that information. This only makes sense for a single
    generator, since the color then no longer identifies the generator.
    """
    if color_by_metric and len(generators) != 1:
        msg = "color_by_metric only works for a single generator"
        raise ValueError(msg)
    _fig, ax = plt.subplots(figsize=PAPER_FIGSIZE)

    colors = ["tab:blue", "tab:orange", "tab:purple", "tab:green"]
    # only used with color_by_metric, one per metric (see below)
    metric_colors = ("tab:blue", "tab:green", "tab:orange")

    # None is the reference simulation, which has no position on the x axis
    settings = [val for val in values if val is not None]

    handles = []
    for idx, generator in enumerate(generators):
        timings = {val: get_timing(generator, name, val) for val in values}

        def per_event(val, key, scale=1, timings=timings):
            return scale * timings[val][key] / timings[val]["n_events"]

        # (key in timing.json, scale, line style, marker, legend label). The
        # wall-clock time is load-dependent, since several scan points run in
        # parallel, so it is the CPU time that should be compared across plots
        metrics = (
            ("cpu_time", 1000, "-", ".", "CPU time"),
            ("wall_time", 1000, "--", "^", "Wall-clock time"),
            ("output_size", 1, ":", "s", "Output size"),
        )

        metric_handles = []
        for m_idx, (key, scale, linestyle, marker, label) in enumerate(metrics):
            reference = per_event(None, key, scale)
            ax.plot(
                settings,
                [100 * per_event(val, key, scale) / reference for val in settings],
                marker=marker,
                markersize=4,
                linestyle=linestyle,
                color=metric_colors[m_idx] if color_by_metric else colors[idx],
            )
            metric_handles.append(
                Line2D(
                    [],
                    [],
                    color=metric_colors[m_idx] if color_by_metric else "black",
                    linestyle=linestyle,
                    marker=marker,
                    label=label.format(reference),
                )
            )

        if color_by_metric:
            handles = metric_handles
        else:
            handles.append(Line2D([], [], color=colors[idx], label=f"{generator}"))

    # the reference is 100 % by construction, for both metrics
    ax.axhline(100, color=DEFAULT_COLOR, linestyle="--", linewidth=1)
    if default_x is None:
        ax.annotate(
            default_label,
            xy=(0.01, 100),
            xycoords=("axes fraction", "data"),
            xytext=(0, -4),
            textcoords="offset points",
            ha="left",
            va="top",
            fontsize=SMALL_FONTSIZE,
            color=DEFAULT_COLOR,
        )
    else:
        mark_default_x(ax, default_x, default_label)
    if mark_x is not None:
        mark_default_x(ax, mark_x, mark_label, color=MARK_COLOR)

    ax.set_xlabel(xlabel if xlabel is not None else f"{name} [{unit}]")
    ax.set_ylabel(f"Relative to {default_label.split(' (')[0].lower()} [%]")
    if title is not None:
        ax.set_title(title, fontsize=TITLE_FONTSIZE)

    # one legend entry per generator (colors) and one per metric (line styles)
    if not color_by_metric:
        handles += metric_handles
    ax.legend(handles=handles, loc="best")

    plt.tight_layout()
    savefig(save_name)


def format_range(low, high, unit="keV"):
    """Format a selection range for a plot title.

    A closed interval is written as ``center ± half-width``, which is easier to
    read than the two edges; an open one keeps the inequality.
    """
    if np.isinf(high):
        return f"> {low:g} {unit}"
    if np.isinf(low):
        return f"< {high:g} {unit}"
    return f"{(low + high) / 2:g} ± {(high - low) / 2:g} {unit}"


def get_bins(list_range, list_binning, e_max=1000):
    # Define bin ranges
    bin_list = []
    for r, b in zip(list_range, list_binning, strict=True):
        bin_list.append(np.arange(r[0] * e_max / 1000, r[1] * e_max / 1000, b))

    return np.unique(np.concatenate(bin_list))


def get_binomial_interval(npass, n):
    eff = npass / n
    quantiles = beta.ppf([0.16, 0.84], npass + 1, n - npass + 1)
    err_low = eff - quantiles[0]
    err_high = quantiles[1] - eff
    if err_high <= 0:
        quantiles = beta.ppf([1 - 0.68, 1], npass + 1, n - npass + 1)
        err_low = eff - quantiles[0]
        err_high = quantiles[1] - eff
    elif err_low <= 0:
        quantiles = beta.ppf([0, 0.68], npass + 1, n - npass + 1)
        err_low = eff - quantiles[0]
        err_high = quantiles[1] - eff
    return err_low, err_high


def normalized_poisson_residual(mu1, mu2) -> np.ndarray:
    if mu1 == 0 or mu2 == 0:
        return 0

    if mu1 > 10 and mu2 > 10:
        return (mu1 - mu2) / np.sqrt(mu1 + mu2)

    samples = poisson.rvs(mu=float(mu1), size=100_000) - poisson.rvs(
        mu=float(mu2), size=100_000
    )
    counts = sum(samples > 0)

    if counts < 5e4:
        sign = -1
        prob = counts / 1e5
    else:
        sign = 1
        counts = 1e5 - counts
        prob = (counts) / 1e5
    if prob == 0:
        prob = 1e-5
    return sign * norm.ppf(1 - prob)


def norm_histo(histo, bins):
    c, bc = histo.to_numpy()
    bc = bc[:-1]
    counts = copy.deepcopy(c)
    for b in range(histo.size - 2):
        histo[b] *= 1 / np.diff(bins)[b]
    return counts, bc


def plot(
    generator,
    xrange,
    values,
    names,
    fields,
    scale="log",
    ylims=None,
    range_zoom=(990, 1010),
    eff_range=(999, 1001),
    dist_range=None,
    doeff=False,
    figsize=(12, 4),
    legend=True,
    n_bins=None,
    label="Energy [keV]",
    save_spec_name="spec.png",
    save_eff_name="eff.png",
    unit="um",
    xlabel=None,
    default_label=None,
    legend_title=None,
    default_x=None,
    eff_title=None,
    mark_x=None,
    mark_label="remage default",
):
    bins_tmp = np.linspace(xrange[0], xrange[1], n_bins) if n_bins is not None else bins

    if dist_range is None:
        dist_low = None
        dist_high = None
    else:
        dist_low = dist_range[0]
        dist_high = dist_range[1]

    if not isinstance(names, list):
        names = list(names)

    effs = {}
    steps = {}
    eff_def = {}
    n_sels = {}
    colors = [
        "tab:blue",
        "tab:orange",
        "tab:purple",
        "tab:green",
        "tab:grey",
        "tab:cyan",
    ]

    # get default
    for field in fields:
        effs[field] = {}
        steps[field] = {}
        n_sels[field] = {}
        _fig, axs = plt.subplots(
            2,
            1,
            gridspec_kw={"height_ratios": [4, 1], "hspace": 0},
            figsize=figsize,
            sharex=True,
        )

        ak_obj, n_sel = get_lh5(
            generator, names[0], None, dist_low=dist_low, dist_high=dist_high
        )
        n_sels[field]["def"] = n_sel

        ak_obj = ak_obj[ak_obj[field] != 0]
        ak_obj[field] = ak_obj[field]

        hist_def = hist.Hist(hist.axis.Variable(bins_tmp)).fill(
            ak_obj[field].to_numpy() + 1e-4
        )

        def_counts, bin_centers = norm_histo(hist_def, bins_tmp)

        eff_def[field] = ak.sum(
            (ak_obj[field] > eff_range[0]) & (ak_obj[field] < eff_range[1])
        )
        ax = axs[0]

        # add a zoom
        if range_zoom is not None:
            ax_inset = inset_axes(
                ax,
                width="100%",
                height="100%",
                bbox_to_anchor=(0.3, 0.5, 0.6, 0.45),
                bbox_transform=ax.transAxes,
            )
            axes_list = [ax, ax_inset]
        else:
            axes_list = [ax]

        for name in names:
            for a in axes_list:
                hist_def.plot(
                    ax=a,
                    yerr=False,
                    flow=None,
                    fill=True,
                    alpha=0.2,
                    color="tab:blue",
                    label=default_label
                    or ("No limits" if names == ["step_limits"] else "Default"),
                )

            effs[field][name] = []
            steps[field][name] = []
            n_sels[field][name] = []

            for idx, val in enumerate(values):
                ak_obj, n_sel = get_lh5(
                    generator, name, val, dist_low=dist_low, dist_high=dist_high
                )

                n_sels[field][name].append(n_sel)

                ak_obj = ak_obj[ak_obj[field] != 0]
                ak_obj[field] = ak_obj[field]

                hist_tmp = hist.Hist(hist.axis.Variable(bins_tmp)).fill(
                    ak_obj[field].to_numpy() + 1e-4
                )

                counts, _ = norm_histo(hist_tmp, bins_tmp)
                if idx == 0:
                    low_counts = counts

                if idx == 0 or idx == len(values) - 2:
                    for a in axes_list:
                        hist_tmp.plot(ax=a, **style, label=f"{val} {unit} ")

                    if legend:
                        axs[0].legend(loc="upper right")
                        axs[0].legend(ncol=1)
                        axs[0].get_legend().set_title(legend_title or name)

                    axs[0].set_yscale(scale)
                    axs[0].set_xlabel(label)
                    axs[0].set_ylabel("counts")
                    axs[0].set_xlim(*xrange)

                    if ylims is not None:
                        axs[0].set_ylim(*ylims)

                steps[field][name].append(val)
                effs[field][name].append(
                    ak.sum(
                        (ak_obj[field] > eff_range[0]) & (ak_obj[field] < eff_range[1])
                    )
                )

            if range_zoom is not None:
                ax_inset.set_yscale(scale)
                ax_inset.set_xlabel(" ")
                ax_inset.set_ylabel(" ")
                ax_inset.set_xlim(*range_zoom)

            plt.tight_layout()

        resid = np.array(
            [
                normalized_poisson_residual(mu, obs)
                for mu, obs in zip(def_counts, low_counts, strict=True)
            ]
        )
        axs[1].axhspan(-3, 3, color="red", alpha=0.2)
        axs[1].axhspan(-2, 2, color="gold", alpha=0.2)
        axs[1].axhspan(-1, 1, color="green", alpha=0.2)

        axs[1].errorbar(bin_centers, resid, fmt=".", color="black")
        axs[1].set_xlabel(label)
        axs[1].set_ylabel("Resid")
        axs[1].set_ylim(
            -max(np.max(abs(resid)), 4.9) - 0.1, +max(np.max(abs(resid)), 4.9) + 0.1
        )
        plt.tight_layout()
        savefig(save_spec_name)
        if not doeff:
            return

    # plot the efficiency
    _fig, ax = plt.subplots(figsize=PAPER_FIGSIZE)
    reference_label = default_label or "Default"
    for idx, field in enumerate(effs.keys()):
        eff_def_low = (
            100 * get_binomial_interval(eff_def[field], n_sels[field]["def"])[0]
        )
        eff_def_high = (
            100 * get_binomial_interval(eff_def[field], n_sels[field]["def"])[1]
        )
        eff_def_val = 100 * eff_def[field] / n_sels[field]["def"]

        if default_x is None:
            # the reference has no position on the x axis (e.g. "no step limit
            # at all"), so it can only be shown as a band spanning the whole
            # axis. Give it a color of its own and label it in place, to make
            # clear that it is not a scan point.
            ax.axhline(y=eff_def_val, linestyle="--", color=DEFAULT_COLOR)
            ax.axhspan(
                ymin=eff_def_val - eff_def_low,
                ymax=eff_def_val + eff_def_high,
                alpha=0.2,
                color=DEFAULT_COLOR,
            )
            ax.annotate(
                reference_label,
                xy=(0.01, eff_def_val),
                xycoords=("axes fraction", "data"),
                xytext=(0, -4),
                textcoords="offset points",
                ha="left",
                va="top",
                fontsize=SMALL_FONTSIZE,
                color=DEFAULT_COLOR,
            )
        else:
            # the reference is a scan point like any other, just draw it there
            ax.errorbar(
                [default_x],
                [eff_def_val],
                yerr=[[eff_def_low], [eff_def_high]],
                fmt="o",
                markersize=4,
                linestyle="none",
                capsize=3,
                color=colors[idx],
                label=reference_label,
                zorder=3,
            )

        for name in effs[field]:
            e = effs[field][name]
            s = steps[field][name]
            err_low = [
                get_binomial_interval(et, nt)[0] * 100
                for et, nt in zip(e, n_sels[field][name], strict=True)
            ]
            err_high = [
                get_binomial_interval(et, nt)[1] * 100
                for et, nt in zip(e, n_sels[field][name], strict=True)
            ]

            ax.errorbar(
                s,
                100 * np.array(e) / np.array(n_sels[field][name]),
                yerr=[err_low, err_high],
                # markers with capped error bars, no line connecting the scan
                # points: they are independent simulations, not a curve
                fmt="o",
                markersize=4,
                linestyle="none",
                capsize=3,
                color=colors[idx],
            )

    if default_x is not None:
        mark_default_x(ax, default_x, reference_label)
    if mark_x is not None:
        mark_default_x(ax, mark_x, mark_label, color=MARK_COLOR)

    ax.set_xlabel(xlabel if xlabel is not None else f"{name} [{unit}]")
    ax.set_ylabel("Fraction of events [%]")
    title = (
        eff_title
        if eff_title is not None
        else f"Fraction of events in {format_range(*eff_range)} ({label})"
    )
    ax.set_title(title, fontsize=TITLE_FONTSIZE)

    plt.tight_layout()
    savefig(save_eff_name)


bins = get_bins(
    [(-2, 2), (2, 10), (10, 50), (50, 950), (950, 980), (980, 998), (998, 1002)],
    [0.5, 2, 10, 50, 10, 2, 0.5],
)
if __name__ == "__main__":
    all_step_limits = [10, 20, 50, 100, 200, None]
    all_prod_cuts = [0.01, 0.02, 0.05, 0.3, 0.5, 0.7, 1, None]

    plot_name = sys.argv[1]

    # plots for the bulk

    scans = (
        (
            all_prod_cuts,
            "prod_cuts",
            "mm",
            ".cuts",
            {
                # remage sets 0.1 mm in the sensitive region (RMGPhysics.cc), so
                # the reference simulation sits on the x axis like a scan point
                "default_x": 0.1,
                "default_label": "remage default (0.1 mm)",
                "xlabel": "Production cut in the sensitive volume [mm]",
                "legend_title": "Production cut",
            },
        ),
        (
            all_step_limits,
            "step_limits",
            "um",
            "",
            {
                # the reference is "no step limit at all", which is not a value
                # on the x axis
                "default_label": "No step limit",
                "xlabel": "Maximum step length in germanium [µm]",
                "legend_title": "Max. step length",
            },
        ),
    )

    for lim, names, unit, suffix, scan_kwargs in scans:
        timing_kwargs = {
            k: v
            for k, v in scan_kwargs.items()
            if k in ("default_x", "default_label", "xlabel")
        }

        plot_timing(
            ["beta_bulk", "beta_surf"],
            names,
            lim,
            unit,
            save_name=f"{plot_name}.timing{suffix}.output.png",
            **timing_kwargs,
        )

        # the same, but only for the bulk setup: this is the common case, and
        # the reduced plot is the one meant for publications
        plot_timing(
            ["beta_bulk"],
            names,
            lim,
            unit,
            save_name=f"{plot_name}.timing{suffix}.bulk.output.png",
            color_by_metric=True,
            title="Bulk events: simulation performance",
            **timing_kwargs,
        )

        plot(
            "beta_bulk",
            (-1, 1020),
            values=lim,
            names=[names],
            fields=["truth_energy"],
            doeff=True,
            unit=unit,
            save_spec_name=f"{plot_name}.bulk-total-energy{suffix}.spec.output.png",
            save_eff_name=f"{plot_name}.bulk-total-energy{suffix}.eff.output.png",
            eff_title="Bulk: total energy in 1000 ± 1 keV",
            **scan_kwargs,
        )

        plot(
            "beta_bulk",
            (-1, 1020),
            values=lim,
            names=[names],
            fields=["active_energy_avg"],
            doeff=True,
            unit=unit,
            save_spec_name=f"{plot_name}.bulk-active-energy{suffix}.spec.output.png",
            save_eff_name=f"{plot_name}.bulk-active-energy{suffix}.eff.output.png",
            eff_title="Bulk: active energy in 1000 ± 1 keV",
            **scan_kwargs,
        )

        plot(
            "beta_bulk",
            (-1, 1020),
            values=lim,
            dist_range=(0, 1),
            names=[names],
            fields=["active_energy_avg"],
            doeff=True,
            unit=unit,
            save_spec_name=f"{plot_name}.tl-active-energy{suffix}.spec.output.png",
            save_eff_name=f"{plot_name}.tl-active-energy{suffix}.eff.output.png",
            eff_title="Depth < 1 mm: active energy in 1000 ± 1 keV",
            **scan_kwargs,
        )

        plot(
            "beta_bulk",
            (-1, 1020),
            values=lim,
            dist_range=(1, np.inf),
            names=[names],
            fields=["active_energy_avg"],
            doeff=True,
            unit=unit,
            save_spec_name=f"{plot_name}.not-tl-active-energy{suffix}.spec.output.png",
            save_eff_name=f"{plot_name}.not-tl-active-energy{suffix}.eff.output.png",
            eff_title="Depth > 1 mm: active energy in 1000 ± 1 keV",
            **scan_kwargs,
        )

        plot(
            "beta_bulk",
            (0, 2),
            values=lim,
            eff_range=(1, np.inf),
            names=[names],
            fields=["r90_avg"],
            doeff=True,
            n_bins=200,
            range_zoom=None,
            label="r90 [mm]",
            unit=unit,
            save_spec_name=f"{plot_name}.bulk-r90{suffix}.spec.output.png",
            save_eff_name=f"{plot_name}.bulk-r90{suffix}.eff.output.png",
            eff_title="Bulk: r90 > 1 mm",
            **scan_kwargs,
        )

        # plots the surface
        plot(
            "beta_surf",
            (-1, 1020),
            values=lim,
            names=[names],
            fields=["truth_energy"],
            doeff=True,
            unit=unit,
            save_spec_name=f"{plot_name}.surf-total-energy{suffix}.spec.output.png",
            save_eff_name=f"{plot_name}.surf-total-energy{suffix}.eff.output.png",
            eff_title="Surface: total energy in 1000 ± 1 keV",
            **scan_kwargs,
        )

        plot(
            "beta_surf",
            (-1, 1020),
            values=lim,
            names=[names],
            fields=["active_energy_avg"],
            doeff=True,
            range_zoom=None,
            eff_range=(300, np.inf),
            unit=unit,
            save_spec_name=f"{plot_name}.surf-active-energy{suffix}.spec.output.png",
            save_eff_name=f"{plot_name}.surf-active-energy{suffix}.eff.output.png",
            eff_title="Surface: active energy > 300 keV",
            **scan_kwargs,
        )

        plot(
            "beta_surf",
            (0, 2),
            values=lim,
            eff_range=(1, np.inf),
            names=[names],
            fields=["max_z_avg"],
            doeff=True,
            n_bins=200,
            range_zoom=None,
            label="Range [mm]",
            unit=unit,
            save_spec_name=f"{plot_name}.surf-max-z{suffix}.spec.output.png",
            save_eff_name=f"{plot_name}.surf-max-z{suffix}.eff.output.png",
            eff_title="Surface: range > 1 mm",
            **scan_kwargs,
        )
