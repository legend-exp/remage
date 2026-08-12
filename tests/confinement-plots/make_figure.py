#!/usr/bin/env python
"""Assemble the confinement examples into one publication-quality figure.

All cases are shown next to each other in a single row: the sampled vertices in
the r-z plane, on top of the outline of the two BEGe detectors.

    $ ./run_all.sh
    $ python make_figure.py            # writes plots/confinement.pdf (and .png)
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

import lh5
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pyg4ometry as pg4
from make_geometry import DETECTOR_PITCH_IN_MM, bege_meta
from pygeomhpges import make_hpge

HERE = Path(__file__).parent

# (file stem, panel title, marker size): the surface cases need slightly larger
# markers, their vertices sit on a line instead of filling an area
CASES = [
    ("union-detectors", "union", 0.4),
    ("lar-cylinder-intersection", "intersection", 0.4),
    ("lar-cylinder-subtraction", "subtraction", 0.4),
    ("detector-surface", "surface", 0.8),
    ("detector-surface-depth", "surface + depth profile", 0.8),
]

# viewport (r, z) in mm, common to all panels
VIEW = ((0, 55), (-40, 40))

A4_WIDTH = 8.27  # inches

VERTEX_COLOR = "#1b6ca8"
OUTLINE_COLOR = "#8a8a8a"
REGION_COLOR = "#c1462b"


def style() -> None:
    """Tweak the matplotlib defaults towards a print figure."""
    font_size = 10.5
    mpl.rcParams.update(
        {
            "font.size": font_size,
            "axes.titlesize": font_size,
            "axes.labelsize": font_size,
            "xtick.labelsize": font_size - 1,
            "ytick.labelsize": font_size - 1,
            "axes.linewidth": 0.8,
            "xtick.major.width": 0.8,
            "ytick.major.width": 0.8,
            "xtick.direction": "in",
            "ytick.direction": "in",
            "xtick.top": True,
            "ytick.right": True,
            "legend.frameon": False,
            "legend.fontsize": 7,
            "pdf.fonttype": 42,
        }
    )


def detector_outlines() -> list[np.ndarray]:
    """(r, z) polygons of the two BEGes, in the coordinates of the world."""
    reg = pg4.geant4.Registry()
    r, z = make_hpge(bege_meta, name="profile", registry=reg).get_profile()
    r, z = np.asarray(r), np.asarray(z)
    # close the polygon and center the profile on z = 0, as in make_geometry.py
    r = np.append(r, r[0])
    z = np.append(z, z[0]) - bege_meta["geometry"]["height_in_mm"] / 2

    return [
        np.column_stack([r, z + sign * DETECTOR_PITCH_IN_MM / 2]) for sign in (+1, -1)
    ]


def sampling_cylinder(macro: Path) -> tuple[float, float, bool] | None:
    """(outer radius, height, excluded) of the geometrical cylinder of a macro."""
    if not macro.exists():
        return None

    text = macro.read_text()

    def value(key: str) -> float | None:
        m = re.search(rf"^/RMG/Generator/Confinement/{key}\s+(\S+)", text, re.MULTILINE)
        return float(m.group(1)) if m else None

    radius = value("Geometrical/Cylinder/OuterRadius")
    height = value("Geometrical/Cylinder/Height")
    if radius is None or height is None:
        return None

    excluded = re.search(r"^/RMG/.*/AddExcludedSolid", text, re.MULTILINE) is not None
    return radius, height, excluded


def read_vertices(
    fn: Path, view, max_points=None, rng=None
) -> tuple[np.ndarray, np.ndarray]:
    """Vertex (r, z) in mm, clipped to the viewport and optionally thinned out."""
    vtx = lh5.read_as("vtx", str(fn), "ak")
    # remage stores vertex coordinates in meters
    x, y, z = (1000 * np.asarray(vtx[k]) for k in ("xloc", "yloc", "zloc"))
    r = np.hypot(x, y)

    (r_lo, r_hi), (z_lo, z_hi) = view
    sel = (r >= r_lo) & (r <= r_hi) & (z >= z_lo) & (z <= z_hi)
    r, z = r[sel], z[sel]

    if max_points is not None and len(r) > max_points:
        idx = rng.choice(len(r), max_points, replace=False)
        r, z = r[idx], z[idx]

    return r, z


def finish_panel(
    ax, view, title, label, subtitle=None, subtitle_color=REGION_COLOR
) -> None:
    """Axis limits, ticks and labelling shared by all panels."""
    ax.set_xlim(*view[0])
    ax.set_ylim(*view[1])
    # the panels are narrow, keep the tick labels from colliding
    ax.xaxis.set_major_locator(mpl.ticker.MultipleLocator(20))
    ax.yaxis.set_major_locator(mpl.ticker.MultipleLocator(20))
    ax.set_aspect("equal")
    ax.set_xlabel("$r$ [mm]")
    # leave room for the extra line of text below the title
    ax.set_title(title, pad=13 if subtitle else 4)
    if subtitle:
        # above the frame: inside the panel the text would sit on top of the
        # vertices and be hard to read
        ax.text(
            0.5,
            1.01,
            subtitle,
            transform=ax.transAxes,
            ha="center",
            va="bottom",
            fontsize=9,
            color=subtitle_color,
        )
    ax.text(
        0.04,
        0.985,
        label,
        transform=ax.transAxes,
        ha="left",
        va="top",
        fontsize=8,
        fontweight="bold",
    )


def draw_panel(
    ax, r, z, view, title, label, outlines, cylinder=None, marker_size=0.4
) -> None:
    # the detector outline goes below the vertices: in the surface-sampling
    # cases the two lie exactly on top of each other
    for poly in outlines:
        ax.plot(poly[:, 0], poly[:, 1], lw=0.7, color=OUTLINE_COLOR, zorder=1)

    ax.scatter(
        r,
        z,
        s=marker_size,
        lw=0,
        alpha=0.35,
        color=VERTEX_COLOR,
        rasterized=True,
        zorder=2,
    )

    if cylinder is not None:
        radius, height, _ = cylinder
        ax.add_patch(
            mpl.patches.Rectangle(
                (0, -height / 2),
                radius,
                height,
                fill=False,
                lw=0.7,
                ls=(0, (4, 2)),
                color=REGION_COLOR,
                zorder=3,
            )
        )
    subtitle = None
    if cylinder is not None:
        subtitle = "virtual cylinder"

    finish_panel(ax, view, title, label, subtitle)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--indir", default=str(HERE / "output"), help="simulation outputs"
    )
    parser.add_argument(
        "--output",
        default=str(HERE / "plots" / "confinement.pdf"),
        help="output figure",
    )
    parser.add_argument(
        "--max-points", type=int, default=12000, help="vertices drawn per panel"
    )
    args = parser.parse_args()

    style()
    rng = np.random.default_rng(42)
    indir = Path(args.indir)
    outlines = detector_outlines()

    # A4 portrait width; the height is only as large as the equal-aspect panels
    # need, so that the figure does not waste vertical space on the page
    panel_width = (A4_WIDTH - 0.9) / len(CASES)
    height = panel_width * (VIEW[1][1] - VIEW[1][0]) / (VIEW[0][1] - VIEW[0][0]) + 0.9

    fig, axes = plt.subplots(
        1, len(CASES), sharey=True, figsize=(A4_WIDTH, height), layout="constrained"
    )

    for ax, (stem, title, marker_size), letter in zip(
        axes, CASES, "abcdefgh", strict=False
    ):
        r, z = read_vertices(indir / f"{stem}.lh5", VIEW, args.max_points, rng)
        draw_panel(
            ax,
            r,
            z,
            VIEW,
            title,
            f"({letter})",
            outlines,
            cylinder=sampling_cylinder(HERE / "macros" / f"{stem}.mac"),
            marker_size=marker_size,
        )
    axes[0].set_ylabel("$z$ [mm]")

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, dpi=400)
    fig.savefig(out.with_suffix(".png"), dpi=300)
    print(f"wrote {out} and {out.with_suffix('.png')}")


if __name__ == "__main__":
    main()
