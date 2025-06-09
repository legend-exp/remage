from __future__ import annotations

import argparse
import sys

import numpy as np
from lgdo import lh5
from matplotlib import pyplot as plt

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["font.size"] = 12

parser = argparse.ArgumentParser(description="Check on LAr subtraction")
parser.add_argument("outfile", type=str, help="File name to process")
parser.add_argument("plot", type=str, help="Where to save the plot")

args = parser.parse_args()

outfile = args.outfile

vertices = lh5.read_as("stp/vtx", outfile, "ak")

# x-y
fig, ax = plt.subplots(figsize=(8, 6))

cmap = plt.cm.BuPu
cmap.set_under("w", 1)
hist = plt.hist2d(
    np.array(1000 * vertices.xloc),
    np.array(1000 * vertices.yloc),
    bins=[100, 100],
    range=[[-300, 300], [-300, 300]],
    vmin=0.5,
    cmap=cmap,
)
ax.set_xlabel("x position [mm]")
ax.set_ylabel("y position [mm]")
ax.axis("equal")

# set the caption
caption = "The x-y positions of the vertices for the simulations of the liquid argon outside imaginary mini-shrouds (per string). "
caption += "The vertices should be excluded from cylinders of radius 44 mm around each string. A higher density of points should be found near the edges."

plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])

plt.savefig(f"{args.plot}-xy.output.png")

# x-z
fig, ax = plt.subplots(1, 1, figsize=(10, 6), sharey=True)

min_x = min(1000 * vertices.xloc)
hist = ax.hist2d(
    np.array(1000 * vertices.xloc),
    np.array(1000 * vertices.zloc),
    bins=[100, 100],
    range=[[-250, 250], [-500, 500]],
    vmin=0.5,
    cmap=cmap,
)
ax.set_xlabel("x position [mm]")

plt.tight_layout()

# set the caption
caption = "The x-z positions of the vertices for the simulations of the liquid argon inside imaginary mini-shrouds (per string). "
caption += (
    "The vertices should be excluded from cylinders (center 50mm and height 400mm)"
)

plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])

plt.savefig(f"{args.plot}-xz.output.png")


sys.exit(0)
