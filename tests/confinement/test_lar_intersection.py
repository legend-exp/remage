from __future__ import annotations

import legendhpges as hpges
import numpy as np
import pyg4ometry as pg4
from lgdo import lh5
from matplotlib import pyplot as plt
from pygeomtools.detectors import get_sensvol_metadata
from scipy import stats

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["font.size"] = 12

# create the registry

reg_tmp = pg4.geant4.Registry()
reg = pg4.gdml.Reader("gdml/ge-array.gdml").getRegistry()

cylinder_height = 400
cylinder_radius = 44

# extract the volume of each lar cylinder
vols = {}
detectors = list(reg.physicalVolumeDict.keys())
detectors = [det for det in detectors if det[0] in ["V", "P", "B", "C"]]


for det in detectors:
    vols[det[0]] = hpges.make_hpge(
        get_sensvol_metadata(reg, f"{det}"), registry=reg_tmp, name=det
    ).volume.magnitude

# information on the detectors
string_radius = 85
string_angles = [0, 80, 150, 220, 290]
det_info = [
    {
        "angle": string_angles[0],
        "radius": string_radius,
        "detectors": ["V", "V", "V", "P"],
        "offsets": [5, 10, 3, 5],
    },
    {
        "angle": string_angles[1],
        "radius": string_radius,
        "detectors": ["B", "B", "B", "B", "B", "B", "P"],
        "offsets": [5, 4, 4, 3, 2, 5, 7],
    },
    {
        "angle": string_angles[2],
        "radius": string_radius,
        "detectors": ["V", "V", "B", "B"],
        "offsets": [10, 8, 7, 2],
    },
    {
        "angle": string_angles[3],
        "radius": string_radius,
        "detectors": ["V", "V", "V"],
        "offsets": [4, 3, 10],
    },
    {
        "angle": string_angles[4],
        "radius": string_radius,
        "detectors": ["B", "C", "C", "V"],
        "offsets": [3, 7, 8, 5],
    },
]

# get the coordinates of the lar cylinders
vol_tot = cylinder_height * np.pi * cylinder_radius**2
x = []
y = []
det_vol = []
for det in det_info:
    angle = det["angle"]
    radius = det["radius"]
    x.append(string_radius * np.sin(np.deg2rad(angle)))
    y.append(string_radius * np.cos(np.deg2rad(angle)))
    det_vol.append(sum([vols[dtype] for dtype in det["detectors"]]))

total_inside = 5 * vol_tot - sum(det_vol)


# read the output
outfile = "test-confine-lar-in.lh5"

vertices = lh5.read_as("vtx", outfile, "ak")

# append which string the vertices are in

strings = np.array(np.full_like(vertices.evtid, -1))

for idx, (xt, yt) in enumerate(zip(x, y)):
    is_in = (
        np.sqrt((1000 * vertices.xloc - xt) ** 2 + (1000 * vertices.yloc - yt) ** 2)
        < cylinder_radius
    )
    strings[is_in] = idx

vertices["strings"] = strings

# make some plots of the coordinates
out_path = "lar-in-check"

# x-y
fig, ax = plt.subplots(figsize=(8, 6))

cmap = plt.cm.BuPu
cmap.set_under("w", 1)
hist = plt.hist2d(
    np.array(1000 * vertices.xloc),
    np.array(1000 * vertices.yloc),
    bins=[100, 100],
    range=[[-150, 150], [-150, 150]],
    vmin=0.5,
    cmap=cmap,
)
ax.set_xlabel("x position [mm]")
ax.set_ylabel("y position [mm]")
ax.axis("equal")

# set the caption
caption = "The x-y positions of the vertices for the simulations of the liquid argon inside imaginary mini-shrouds (per string). "
caption += "The vertices should be arranged in cylinders of radius 44 mm around each string. A higher density of points should be found near the edges."

plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])

plt.savefig(f"{out_path}-xy.output.png")

# x-z
fig, ax = plt.subplots(1, 5, figsize=(10, 6), sharey=True)
for s in range(5):
    min_x = min(1000 * vertices.xloc[vertices.strings == s])
    hist = ax[s].hist2d(
        np.array(1000 * vertices.xloc[vertices.strings == s]),
        np.array(1000 * vertices.zloc[vertices.strings == s]),
        bins=[100, 100],
        range=[[-150, 150], [-160, 260]],
        vmin=0.5,
        cmap=cmap,
    )
    ax[s].set_xlabel("x position [mm]")
    if s == 0:
        ax[s].set_ylabel("z position [mm]")
plt.tight_layout()

# set the caption
caption = "The x-z positions of the vertices for the simulations of the liquid argon inside imaginary mini-shrouds (per string). "
caption += "The vertices should be arranged in cylinders (center 50mm and height 400mm) and the regions with HPGe should be visible."

plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])

plt.savefig(f"{out_path}-xz.output.png")

# now plot the fraction in each detector
fraction_vert = []
fraction_vert_err = []
names = []
expected_fraction = []
vert = vertices
n_tot = len(vert)

for s, v in enumerate(det_vol):
    n_sel_vert = len(vert[vert.strings == s])
    fraction_vert.append(100 * n_sel_vert / n_tot)
    fraction_vert_err.append(100 * np.sqrt(n_sel_vert) / n_tot)
    expected_fraction.append(100 * (vol_tot - v) / total_inside)

fraction_vert = np.array(fraction_vert)
fraction_vert_err = np.array(fraction_vert_err)
expected_fraction = np.array(expected_fraction)

# get the p-value

test_stat = 2 * np.sum(
    n_tot * expected_fraction / 100
    - (fraction_vert * n_tot / 100) * (1 - np.log(fraction_vert / expected_fraction))
)

# should follow a chi2 distribution with N -1 dof

N = len(det_vol)
p = stats.chi2.sf(test_stat, N - 1)
sigma = stats.norm.ppf(1 - p)

# make the plot
fig, ax = plt.subplots(2, 1, figsize=(12, 6), sharex=True)

ax[0].errorbar(
    np.arange(5),
    fraction_vert,
    yerr=fraction_vert_err,
    fmt=".",
    label="Vertices",
)
ax[0].errorbar(
    np.arange(5),
    expected_fraction,
    fmt="x",
    label="Expected",
)

ax[0].set_ylabel("Fraction of vertices [%]")
ax[0].set_xticks(
    np.arange(5), [f"string {i}" for i in range(5)], rotation=90, fontsize=13
)
ax[0].legend()
ax[0].grid()

# make the residual
ax[1].errorbar(
    np.arange(5),
    100 * (fraction_vert - expected_fraction) / expected_fraction,
    yerr=100 * fraction_vert_err / expected_fraction,
    fmt=".",
)

ax[1].set_ylabel("Relative Difference [%]")

ax[0].set_xticks(
    np.arange(5), [f"string {i}" for i in range(5)], rotation=90, fontsize=13
)
ax[1].axhline(y=0, color="red")
ax[1].grid()
fig.suptitle(f"Intersection check ({sigma:.1f} $\sigma$)")

caption = "The fraction of the vertices found inside each cylinder. This is compared to the expectation which is that the number "
caption += "should be proportional to the volume of the cylinder minus the total HPGe volume. The top panel shows the fraction in each string "
caption += (
    r"while the lower panel shows the relative difference in % from the expectation"
)
plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])

plt.savefig(f"{out_path}-ratios.output.png")

assert sigma < 5
