from __future__ import annotations

import argparse
import copy

import numpy as np
import pyg4ometry as pg4
from lgdo import lh5
from matplotlib import pyplot as plt
from scipy import stats

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["font.size"] = 12


parser = argparse.ArgumentParser(description="Check on surface generation")
parser.add_argument("det", type=str, help="detector type")

args = parser.parse_args()


gdml = "gdml/simple-solids.gdml"

reg = pg4.gdml.Reader(gdml).getRegistry()


def add_local_pos(vertices, pos):
    xlocal = np.array(1000 * vertices.xloc.to_numpy())
    ylocal = np.array(1000 * vertices.yloc.to_numpy())
    zlocal = np.array(1000 * vertices.zloc.to_numpy())

    vertices["xlocal"] = xlocal - pos[0]
    vertices["ylocal"] = ylocal - pos[1]
    vertices["zlocal"] = zlocal - pos[2]
    return vertices


tol = 1e-6
select_sides = {
    "tubby": {
        "func": [
            lambda x, y, z: (abs(z - 30) < tol) & (np.sqrt(x**2 + y**2) < 60),  # bottom
            lambda x, y, z: (abs(np.sqrt(x**2 + y**2) - 60) < tol)
            & (abs(z) < 30 - tol),  # side
            lambda x, y, z: (abs(z + 30) < tol) & (np.sqrt(x**2 + y**2) < 60),  # top
        ],
        "area": [
            np.pi * 60**2,
            2 * np.pi * 60 * 60,
            np.pi * 60**2,
        ],
        "order": [0, 1, 2],
        "nice_name": "G4Tubs",
    },
    "sub": {
        "func": [
            lambda x, y, z: (abs(z + 30) < tol)
            & (np.sqrt(x**2 + y**2) < 60)
            & (np.sqrt(x**2 + y**2) > 20),  # bottom
            lambda x, y, z: (abs(z - 30) < tol)
            & (np.sqrt(x**2 + y**2) < 60)
            & (np.sqrt(x**2 + y**2) > 20),  # top
            lambda x, y, z: (abs(np.sqrt(x**2 + y**2) - 60) < tol)
            & (abs(z) < 30 - tol),  # outside
            lambda x, y, z: (abs(np.sqrt(x**2 + y**2) - 20) < tol)
            & (abs(z) < 30 - tol),  # inside
        ],
        "area": [
            np.pi * (60**2 - 20**2),  # bottom
            np.pi * (60**2 - 20**2),  # top
            2 * np.pi * 60 * 60,  # outside
            2 * np.pi * 60 * 20,
        ],  # inside
        "order": [1, 2, 3, 0],
        "nice_name": "subtraction of two G4Tubs",
    },
    "con": {
        "func": [
            lambda x, y, z: (abs(z + 50) < tol) & (np.sqrt(x**2 + y**2) < 60),  # bottom
            lambda x, y, z: (abs(z + 100 * (np.sqrt(x**2 + y**2) / 60) - 50) < tol)
            & (abs(z) < 50 - tol),  # side,
        ],
        "area": [
            np.pi * 60**2,  # bottom
            np.pi * 60 * np.sqrt(60**2 + 100**2),  # side
        ],
        "order": [1, 0],
        "nice_name": "G4Cone",
    },
    "box": {
        "func": [
            lambda x, y, z: (abs(z - 50) < tol) & (abs(x) < 25) & (abs(y) < 25),  # top
            lambda x, y, z: (abs(x - 25) < tol)
            & (abs(y) < 25)
            & (abs(z) < 50 - tol),  # sides
            lambda x, y, z: (abs(y - 25) < tol) & (abs(x) < 25) & (abs(z) < 50 - tol),
            lambda x, y, z: (abs(x + 25) < tol) & (abs(y) < 25) & (abs(z) < 50 - tol),
            lambda x, y, z: (abs(y + 25) < tol) & (abs(x) < 25) & (abs(z) < 50 - tol),
            lambda x, y, z: (abs(z + 50) < tol) & (abs(x) < 25) & (abs(y) < 25),
        ],  # bottom
        "area": [50**2, 100 * 50, 100 * 50, 100 * 50, 100 * 50, 50**2],
        "order": [0, 1, 2, 3, 4, 5],
        "nice_name": "G4Box",
    },
    "uni": {
        "func": [
            lambda x, y, z: (abs(z + 50) < tol)
            & (abs(x) < 25)
            & (abs(y) < 25),  # bottom
            lambda x, y, z: (abs(x - 25) < tol)
            & (abs(y) < 25)
            & (abs(z) < 50 - tol),  # sides
            lambda x, y, z: (abs(y - 25) < tol) & (abs(x) < 25) & (abs(z) < 50 - tol),
            lambda x, y, z: (abs(x + 25) < tol) & (abs(y) < 25) & (abs(z) < 50 - tol),
            lambda x, y, z: (abs(y + 25) < tol) & (abs(x) < 25) & (abs(z) < 50 - tol),
            lambda x, y, z: (abs(z - 50) < tol)
            & (abs(x) < 25)
            & (abs(y) < 25)
            & ((abs(x) > 10) | (abs(y) > 10)),  # top
            lambda x, y, z: (abs(x - 10) < tol)
            & (abs(y) < 10)
            & (abs(z - 75) < 25),  # small sides
            lambda x, y, z: (abs(y - 10) < tol) & (abs(x) < 10) & (abs(z - 75) < 25),
            lambda x, y, z: (abs(x + 10) < tol) & (abs(y) < 10) & (abs(z - 75) < 25),
            lambda x, y, z: (abs(y + 10) < tol) & (abs(x) < 10) & (abs(z - 75) < 25),
            lambda x, y, z: (abs(z - 100) < tol) & (abs(x) < 10) & (abs(y) < 10),
        ],
        "area": [
            50 * 50.0,
            100 * 50.0,
            100 * 50.0,
            100 * 50.0,
            100 * 50.0,
            50 * 50.0 - 20 * 20.0,
            20 * 50.0,
            20 * 50.0,
            20 * 50.0,
            20 * 50.0,
            20 * 20.0,
        ],
        "order": [5, 6, 7, 8, 9, 10, 1, 2, 3, 4, 0],
        "nice_name": "Union of two G4Boxs",
    },
    "trd": {
        "func": [
            lambda x, y, z: (abs(z + 50) < tol) & (abs(x) < 25) & (abs(y) < 25),  # base
            lambda x, y, z: (abs(z - 50) < tol) & (abs(x) < 5) & (abs(y) < 5),  # top
            lambda x, y, z: (y > 5)
            & (y < 25)
            & (y > x)
            & (y > -x)
            & (abs(z) < 50 - tol),  # sides
            lambda x, y, z: (-y > 5)
            & (-y < 25)
            & (-y > x)
            & (-y > -x)
            & (abs(z) < 50 - tol),
            lambda x, y, z: (x > 5)
            & (x < 25)
            & (x > y)
            & (x > -y)
            & (abs(z) < 50 - tol),
            lambda x, y, z: (-x > 5)
            & (-x < 25)
            & (-x > y)
            & (-x > -y)
            & (abs(z) < 50 - tol),
        ],
        "area": [
            50 * 50.0,
            10 * 10.0,
            np.sqrt(20 * 20 + 100 * 100) * (10 + 50) / 2.0,
            np.sqrt(20 * 20 + 100 * 100) * (10 + 50) / 2.0,
            np.sqrt(20 * 20 + 100 * 100) * (10 + 50) / 2.0,
            np.sqrt(20 * 20 + 100 * 100) * (10 + 50) / 2.0,
        ],
        "order": [1, 2, 4, 3, 5, 0],
        "nice_name": "G4Trapezoid",
    },
}
dtype = args.det

out_path = f"confinement.simple-solids-surface-{dtype}"
# get positions
pos = reg.physicalVolumeDict[dtype].position.eval()

# read vertices and hits
outfile = f"test-confine-surface-{dtype}-out.lh5"
vertices = lh5.read_as("stp/vertices", outfile, "ak")
hits = lh5.read_as("stp/germanium", outfile, "ak")

hits = add_local_pos(hits, pos)
vertices = add_local_pos(vertices, pos)
indices = np.array(np.full_like(vertices.time, -1))

# search for vertices being close to the sides
funcs = select_sides[dtype]["func"]
nice_name = select_sides[dtype]["nice_name"]

for idx, fun in enumerate(funcs):
    x = vertices.xlocal
    y = vertices.ylocal
    z = vertices.zlocal
    is_close = fun(
        np.array(vertices.xlocal.to_numpy()),
        np.array(vertices.ylocal.to_numpy()),
        np.array(vertices.zlocal.to_numpy()),
    )
    indices[is_close] = idx

if len(indices[indices == -1]) > 0:
    msg = f"{dtype} has primaries not close to any side"
    raise ValueError(msg)

vertices["idx"] = indices
vertices["rlocal"] = np.sqrt(vertices.xlocal**2 + vertices.ylocal**2)
n = 5000
vert_small = vertices[0:n]

# 3D plot
fig = plt.figure(constrained_layout=True, figsize=(8, 6))
ax = fig.add_subplot(111, projection="3d", computed_zorder=False)

order = copy.copy(select_sides[dtype]["order"])
order.reverse()

for idx in range(len(funcs)):
    sel = vert_small.idx == order[idx]
    scatter = ax.scatter(
        vert_small[sel].xlocal,
        vert_small[sel].ylocal,
        vert_small[sel].zlocal,
        zorder=idx,
        s=5,
    )

sel = vert_small.idx == -1
scatter = ax.scatter(
    vert_small[sel].xlocal, vert_small[sel].ylocal, vert_small[sel].zlocal, s=5
)
ax.view_init(elev=20, azim=30)

ax.set_xlabel("x position [mm]")
ax.set_ylabel("y position [mm]")
ax.set_zlabel("z position [mm]")

caption = f"The position of primary vertices for {nice_name} in 3D space. \n"
caption += "Primaries are grouped by the surface with each shown in a different color. "

plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])
plt.savefig(f"{out_path}-3d.output.png")

# 2D plots
fig, ax = plt.subplots(1, 3, figsize=(8, 6))
for idx, var in enumerate(
    [("xlocal", "ylocal"), ("rlocal", "zlocal"), ("xlocal", "zlocal")]
):
    for idx2 in range(len(funcs)):
        sel = vert_small.idx == order[idx2]
        ax[idx].scatter(vert_small[sel][var[0]], vert_small[sel][var[1]], s=2)

    sel = vert_small.idx == -1
    ax[idx].scatter(vert_small[sel][var[0]], vert_small[sel][var[1]], s=2)

    ax[idx].set_xlabel(f" {var[0]} [mm]")
    ax[idx].set_ylabel(f" {var[1]} [mm]")
    ax[idx].axis("equal")
    plt.tight_layout()

caption = f"The position of primary vertices for {nice_name} in 2D space for different projections. \n"
caption += "Primaries are grouped by the surface with each shown in a different color. "

plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])
plt.savefig(f"{out_path}-projections.output.png")

# statistical test
fraction_vert = []
fraction_vert_err = []

names = []
expected_fraction = []
vert = vertices
n_tot = len(vertices)
tot = sum(select_sides[dtype]["area"])

for idx, a in enumerate(select_sides[dtype]["area"]):
    n_sel_vert = len(vertices[vertices.idx == idx])
    fraction_vert.append(100 * n_sel_vert / n_tot)
    fraction_vert_err.append(100 * np.sqrt(n_sel_vert) / n_tot)
    expected_fraction.append(100 * a / tot)

fraction_vert = np.array(fraction_vert)
fraction_vert_err = np.array(fraction_vert_err)
expected_fraction = np.array(expected_fraction)

# get the p-value

test_stat = 2 * np.sum(
    n_tot * expected_fraction / 100
    - (fraction_vert * n_tot / 100) * (1 - np.log(fraction_vert / expected_fraction))
)

# should follow a chi2 distribution with N -1 dof

N = len(select_sides[dtype]["area"])
p = stats.chi2.sf(test_stat, N - 1)
sigma = stats.norm.ppf(1 - p)

# make the plot
fig, ax = plt.subplots(2, 1, figsize=(8, 6), sharex=True)

ax[0].errorbar(
    np.arange(N),
    fraction_vert,
    yerr=fraction_vert_err,
    fmt=".",
    label="Vertices",
)
ax[0].errorbar(
    np.arange(N),
    expected_fraction,
    fmt="x",
    label="Expected",
)

ax[0].set_ylabel("Fraction of vertices [%]")
ax[0].set_xticks(
    np.arange(N), [f"face {i}" for i in range(N)], rotation=90, fontsize=13
)
ax[0].legend()
ax[0].grid()

# make the residual
ax[1].errorbar(
    np.arange(N),
    100 * (fraction_vert - expected_fraction) / expected_fraction,
    yerr=100 * fraction_vert_err / expected_fraction,
    fmt=".",
)

ax[1].set_ylabel("Relative Difference [%]")

ax[0].set_xticks(
    np.arange(N), [f"face {i}" for i in range(N)], rotation=90, fontsize=13
)
ax[1].axhline(y=0, color="red")
ax[1].grid()
fig.suptitle(f"Surface uniformity test for {nice_name} ({sigma:.1f} $\\sigma$)")

caption = "The fraction of the vertices found on each face of the shapes."
caption += "This should be proportional to the surface area. The top panel shows the fraction in each face "
caption += (
    r"while the lower panel shows the relative difference in % from the expectation"
)
plt.figtext(0.1, 0.06, caption, wrap=True, ha="left", fontsize=11)
plt.tight_layout(rect=[0, 0.12, 1, 1])
plt.savefig(f"{out_path}-ratios.output.png")

assert sigma < 5
