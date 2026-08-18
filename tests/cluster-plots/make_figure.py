#!/usr/bin/env python
"""Visualise the effect of the step pre-clustering on the recorded steps.

The same events are simulated twice, with and without pre-clustering, and the
most energetic interaction site of every event is shown side by side, one
event per page:

    $ python make_figure.py --indir output --output clustering.pdf
"""

from __future__ import annotations

import argparse
from pathlib import Path

import awkward as ak
import lh5
import numpy as np
from matplotlib import pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.lines import Line2D

HERE = Path(__file__).parent

plt.rcParams["font.size"] = 11
plt.rcParams["lines.linewidth"] = 1

# debugging aid: annotate the creation process of every secondary track
SHOW_PROCESS = False

# placement of the two detectors along z (mm)
DETECTORS = {"det001": 5.0, "det002": -27.0}

# colour code of the particles depositing the energy: for each PDG code a label
# and a list of (initial kinetic energy range in keV, colour). The ranges are
# tried in order, ``None`` matches any energy and ``None`` as an interval bound
# means open-ended; steps of tracks that match no range (or whose track is not
# stored) fall back to the last colour.
PARTICLES = {
    11: ("$e^-$", ((None, "tab:blue"),)),
    -11: ("$e^+$", ((None, "tab:cyan"),)),
    22: (r"$\gamma$", (((100, None), "tab:red"), ((None, 100), "tab:orange"))),
}


def read_steps(fname):
    """Read all germanium steps of both detectors into a flat table (mm, keV).

    The output is read in its reshaped (jagged) form, so that the track IDs are
    available; the rows are flattened here, keeping the event and track
    association in dedicated columns. The initial kinetic energy of the track a
    step belongs to is attached as an extra column (NaN if the track is not in
    the ``tracks`` table).
    """
    parts = []
    for det in DETECTORS:
        data = lh5.read_as(f"stp/{det}", fname, "ak", with_units=True)
        for name in ("xloc", "yloc", "zloc"):
            assert ak.parameters(data[name])["units"] == "m"

        # broadcast the per-event evtid onto the steps, then flatten everything
        evtid = ak.broadcast_arrays(data.evtid, data.edep)[0]
        parts.append(
            ak.Array(
                {
                    "evtid": ak.flatten(evtid),
                    "trackid": ak.flatten(data.trackid),
                    "particle": ak.flatten(data.particle),
                    "edep": ak.flatten(ak.values_astype(data.edep, np.float64)),
                    "time": ak.flatten(data.time),
                    "x": 1e3 * ak.flatten(ak.values_astype(data.xloc, np.float64)),
                    "z": 1e3 * ak.flatten(ak.values_astype(data.zloc, np.float64)),
                }
            )
        )
    steps = ak.concatenate(parts)

    ekin = read_track_energies(fname)
    return ak.with_field(
        steps,
        np.array(
            [
                ekin.get((int(e), int(t)), np.nan)
                for e, t in zip(
                    ak.to_numpy(steps.evtid), ak.to_numpy(steps.trackid), strict=True
                )
            ]
        ),
        "ekin",
    )


def read_track_energies(fname):
    """Initial kinetic energy (keV) of every stored track, as ``{(evtid, trackid): ekin}``."""
    trk = lh5.read_as("tracks", fname, "ak", with_units=True)
    assert ak.parameters(trk.ekin)["units"] == "MeV"
    return {
        (int(e), int(t)): 1e3 * float(k)
        for e, t, k in zip(trk.evtid, trk.trackid, trk.ekin, strict=True)
    }


def read_vertices(fname):
    """Read the primary vertices (mm), indexed by event ID."""
    vtx = lh5.read_as("vtx", fname, "ak", with_units=True)
    assert ak.parameters(vtx.xloc)["units"] == "m"
    return {
        int(e): (1e3 * float(x), 1e3 * float(z))
        for e, x, z in zip(vtx.evtid, vtx.xloc, vtx.zloc, strict=True)
    }


def read_track_origins(fname):
    """Creation vertex of every stored track as ``{(evtid, trackid): (x, z, process)}``."""
    procs = lh5.read("processes", fname)
    names = {int(procs[k].value): k for k in procs}

    trk = lh5.read_as("tracks", fname, "ak", with_units=True)
    assert ak.parameters(trk.xloc)["units"] == "m"
    return {
        (int(e), int(t)): (1e3 * float(x), 1e3 * float(z), names.get(int(pid)))
        for e, t, x, z, pid in zip(
            trk.evtid, trk.trackid, trk.xloc, trk.zloc, trk.procid, strict=True
        )
    }


def marker_size(edep, scale=2000, floor=6):
    """Marker area proportional to edep, floored so that tiny steps stay visible."""
    return floor + scale * np.asarray(edep) / 1000.0


def color_index(ekin, colors):
    """Index into ``colors`` of the range each initial energy (keV) falls into.

    The ranges are tried in order; whatever is left over (no matching range, or
    an unknown energy) is assigned to the last entry.
    """
    ekin = np.asarray(ekin, dtype=np.float64)
    index = np.full(len(ekin), len(colors) - 1, dtype=int)
    todo = np.ones(len(ekin), dtype=bool)
    for i, (erange, _color) in enumerate(colors):
        if erange is None:
            match = todo
        else:
            low, high = erange
            match = todo.copy()
            if low is not None:
                match &= ekin >= low
            if high is not None:
                match &= ekin < high
        index[match] = i
        todo &= ~match
    return index


def track_color(pdg, ekin):
    """Colour of a track of particle ``pdg`` with initial kinetic energy ``ekin`` (keV)."""
    colors = PARTICLES[pdg][1]
    return colors[color_index([ekin], colors)[0]][1]


def range_label(label, erange):
    """``label``, annotated with the initial-energy range it stands for."""
    if erange is None:
        return label
    low, high = erange
    if low is None:
        return f"{label} ($E_0 < {high:g}$ keV)"
    if high is None:
        return f"{label} ($E_0 > {low:g}$ keV)"
    return f"{label} (${low:g} < E_0 < {high:g}$ keV)"


def draw_tracks(ax, data, vertex=None, origins=None):
    """Draw one line per track ID, in the colour of the particle of that track.

    The steps of a track are connected in time order, starting at the creation
    vertex of the track (from ``origins``, if given), so that also tracks with a
    single recorded step show up as a line. The creation vertices of the
    secondary tracks are marked with an open circle. If ``vertex`` is given, the
    generated primary vertex is marked as well.
    """
    for trackid in np.unique(ak.to_numpy(data.trackid)):
        track = data[data.trackid == trackid]
        track = track[ak.argsort(track.time)]
        pdg = int(track.particle[0])
        if pdg not in PARTICLES:
            continue
        color = track_color(pdg, float(track.ekin[0]))

        xs, zs = ak.to_numpy(track.x), ak.to_numpy(track.z)
        origin = None if origins is None else origins.get(int(trackid))
        process = origin[2] if origin is not None else None
        origin = origin[:2] if origin is not None else None
        if origin is None and vertex is not None and trackid == 1:
            origin = vertex
        if origin is not None:
            xs = np.concatenate([[origin[0]], xs])
            zs = np.concatenate([[origin[1]], zs])

        if len(xs) > 1:
            ax.plot(
                xs,
                zs,
                color=color,
                lw=1.2,
                alpha=1,
                zorder=1,
                solid_capstyle="round",
            )

        # debugging aid: name of the process that created this secondary
        if origin is not None and process is not None and SHOW_PROCESS:
            ax.annotate(
                process,
                origin,
                textcoords="offset points",
                xytext=(3, 2),
                fontsize=4,
                color=color,
                zorder=4,
            )


def scatter_steps(ax, data, tracks=True, vertex=None, origins=None):
    """Scatter the steps of ``data``, colour-coded by particle and initial energy."""
    if tracks:
        draw_tracks(ax, data, vertex=vertex, origins=origins)

    for pdg, (_label, colors) in PARTICLES.items():
        part = data[data.particle == pdg]
        if len(part) == 0:
            continue

        # the colour is a property of the track, not of the step: split the
        # steps by the energy range the track they belong to started in
        index = color_index(ak.to_numpy(part.ekin), colors)
        for i, (_erange, color) in enumerate(colors):
            sel = part[index == i]
            if len(sel) == 0:
                continue
            ax.scatter(
                ak.to_numpy(sel.x),
                ak.to_numpy(sel.z),
                s=marker_size(ak.to_numpy(sel.edep)),
                color=color,
                edgecolor="white",
                lw=0.4,
                alpha=0.5,
                zorder=2,
            )


def legend_handles(pdgs=None):
    handles = []
    for pdg, (label, colors) in PARTICLES.items():
        if pdgs is not None and pdg not in pdgs:
            continue
        for erange, color in colors:
            handles.append(
                Line2D(
                    [],
                    [],
                    ls="",
                    marker="o",
                    markersize=7,
                    color=color,
                    markeredgecolor="white",
                    label=range_label(label, erange),
                )
            )

    handles.append(
        Line2D(
            [],
            [],
            ls="",
            marker="o",
            markersize=7,
            markerfacecolor="none",
            color="0.4",
            label="area $\\propto E_\\mathrm{dep}$",
        )
    )
    return handles


def style_axes(ax, xlim=(-25, 25), zlim=(-5, 37)):
    ax.set_aspect("equal")
    ax.set_xlim(*xlim)
    ax.set_ylim(*zlim)
    ax.set_xlabel("x [mm]")
    ax.set_ylabel("z [mm]")


def _bbox(data):
    """(x, z) bounding box of a set of steps."""
    x, z = ak.to_numpy(data.x), ak.to_numpy(data.z)
    return (float(x.min()), float(x.max())), (float(z.min()), float(z.max()))


def _union(box_a, box_b):
    return (
        (min(box_a[0][0], box_b[0][0]), max(box_a[0][1], box_b[0][1])),
        (min(box_a[1][0], box_b[1][0]), max(box_a[1][1], box_b[1][1])),
    )


def _size(box):
    return max(box[0][1] - box[0][0], box[1][1] - box[1][0])


def _nearest_gamma(data, center):
    """The gamma step of ``data`` closest to ``center``, as a bounding box."""
    gam = data[data.particle == 22]
    if len(gam) == 0:
        return None
    x, z = ak.to_numpy(gam.x), ak.to_numpy(gam.z)
    i = int(np.argmin(np.hypot(x - center[0], z - center[1])))
    return ((float(x[i]), float(x[i])), (float(z[i]), float(z[i])))


def zoom_window(datasets, evtid, search=1.0, pad=0.3, minimum=0.04, max_growth=4.0):
    """Window (in mm) around the most energetic deposition of an event.

    The window covers the steps of that interaction site in all ``datasets``,
    so that the sub-millimetre structure of the electron track becomes visible.
    It is grown to also include the closest gamma interaction vertex — the
    origin of the electron — unless that would enlarge the window by more than
    ``max_growth``, in which case the electron structure takes precedence.
    """
    ref = datasets[0][datasets[0].evtid == evtid]
    imax = ak.argmax(ref.edep)
    center = (float(ref.x[imax]), float(ref.z[imax]))

    sites = []
    for data in datasets:
        ev = data[data.evtid == evtid]
        site = ev[(abs(ev.x - center[0]) < search) & (abs(ev.z - center[1]) < search)]
        if len(site) > 0:
            sites.append(site)

    # the box that must be resolved: the charged-particle steps of the site
    box = None
    for site in sites:
        charged = site[site.particle != 22]
        sub = _bbox(charged if len(charged) > 0 else site)
        box = sub if box is None else _union(box, sub)

    # grow it towards the gamma vertex the electron originates from
    grown = box
    for site in sites:
        gamma = _nearest_gamma(site, center)
        if gamma is not None:
            grown = _union(grown, gamma)
    if _size(grown) <= max_growth * max(_size(box), minimum):
        box = grown

    xc = 0.5 * (box[0][0] + box[0][1])
    zc = 0.5 * (box[1][0] + box[1][1])
    half = max(0.5 * (1 + 2 * pad) * _size(box), minimum)
    return (xc - half, xc + half), (zc - half, zc + half)


def add_step_count(ax, lims, data):
    """Annotate the number of steps falling inside the plotted window."""
    if len(data) == 0:
        return
    x, z = ak.to_numpy(data.x), ak.to_numpy(data.z)
    n = int(
        np.sum(
            (x >= lims[0][0])
            & (x <= lims[0][1])
            & (z >= lims[1][0])
            & (z <= lims[1][1])
        )
    )
    ax.text(
        0.03,
        0.96,
        f"{n} step" + ("s" if n != 1 else ""),
        transform=ax.transAxes,
        ha="left",
        va="top",
        fontsize=8,
        bbox={"facecolor": "white", "alpha": 0.7, "pad": 1.5, "edgecolor": "none"},
        zorder=5,
    )


def add_scale_bar(ax, lims, data=None):
    """Draw a scale bar with a round length into the emptier bottom corner."""
    width = lims[0][1] - lims[0][0]
    length = max(x for x in (0.01, 0.02, 0.05, 0.1, 0.2, 0.5) if x < 0.45 * width)

    left = True
    if data is not None and len(data) > 0:
        x, z = ak.to_numpy(data.x), ak.to_numpy(data.z)
        low = z < lims[1][0] + 0.35 * width
        n_left = int(np.sum(low & (x < lims[0][0] + 0.5 * width)))
        n_right = int(np.sum(low & (x >= lims[0][0] + 0.5 * width)))
        left = n_left <= n_right

    x0 = lims[0][0] + (0.08 if left else 0.92 - length / width) * width
    z0 = lims[1][0] + 0.09 * width
    ax.plot([x0, x0 + length], [z0, z0], color="black", lw=1.6, solid_capstyle="butt")
    ax.text(
        x0 + length / 2,
        z0 + 0.04 * width,
        f"{length * 1e3:.0f}" + r" $\mu$m",
        ha="center",
        va="bottom",
        fontsize=7,
        bbox={"facecolor": "white", "alpha": 0.7, "pad": 0.8, "edgecolor": "none"},
    )


def plot_event(clu, noclu, vertices, evtid, origins=None):
    """Publication figure of a single event: left un-clustered, right clustered.

    Both panels show the same x-z view of the two detectors; the most energetic
    interaction site, where the individual steps of the electron track would
    otherwise fall onto a single marker.
    """
    fig, axes = plt.subplots(
        1, 2, figsize=(7.2, 2.5), sharex=True, sharey=True, layout="constrained"
    )
    fig.get_layout_engine().set(rect=[0.2, 0, 0.85, 1])
    vertex = vertices.get(int(evtid))
    lims = zoom_window((noclu, clu), evtid)

    noclu_org, clu_org = origins if origins is not None else ({}, {})
    for ax, panel, (data, org, title) in zip(
        axes,
        ("a", "b"),
        (
            (noclu, noclu_org, "no clustering"),
            (clu, clu_org, "with clustering"),
        ),
        strict=True,
    ):
        sel = data[data.evtid == evtid]
        starts = {t: xz for (e, t), xz in org.items() if e == int(evtid)}

        scatter_steps(ax, sel, vertex=vertex, origins=starts)
        style_axes(ax)
        ax.set_title(f"$\\bf{{({panel})}}$ {title}", fontsize=10)

        ax.set_xlim(*lims[0])
        ax.set_ylim(*lims[1])
        ax.set_aspect("equal")
        ax.set_xlabel("")
        ax.set_ylabel("")
        ax.set_xticks([])
        ax.set_yticks([])
        for spine in ax.spines.values():
            spine.set_color("black")
            spine.set_linewidth(1.0)
        add_scale_bar(ax, lims, data=sel)
        add_step_count(ax, lims, sel)

    axes[1].set_ylabel("")
    fig.legend(
        handles=legend_handles(pdgs=set(ak.to_numpy(noclu.particle))),
        fontsize=9,
        ncol=1,
        loc="center left",
        # frameon=False,
        handletextpad=0.4,
        columnspacing=1.4,
    )

    return fig


def select_events(clu, noclu):
    """All events that deposited energy in both datasets, in ID order.

    Only events present in both can be compared side by side.
    """
    return np.intersect1d(ak.to_numpy(clu.evtid), ak.to_numpy(noclu.evtid))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--indir", default=str(HERE / "output"), help="simulation outputs"
    )
    parser.add_argument(
        "--output", default=str(HERE / "plots" / "clustering.pdf"), help="output figure"
    )
    args = parser.parse_args()

    indir = Path(args.indir)
    clu = read_steps(str(indir / "cluster.lh5"))
    noclu = read_steps(str(indir / "nocluster.lh5"))
    vertices = read_vertices(str(indir / "nocluster.lh5"))
    origins = (
        read_track_origins(str(indir / "nocluster.lh5")),
        read_track_origins(str(indir / "cluster.lh5")),
    )

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)

    # one event per page, the first one also as a stand-alone bitmap
    with PdfPages(out) as pdf:
        for i, evtid in enumerate(select_events(clu, noclu)):
            fig = plot_event(clu, noclu, vertices, evtid, origins=origins)
            pdf.savefig(fig, dpi=300)
            if i == 0:
                fig.savefig(out.with_suffix(".png"), dpi=300)
            plt.close(fig)

    print(f"wrote {out} and {out.with_suffix('.png')}")


if __name__ == "__main__":
    main()
