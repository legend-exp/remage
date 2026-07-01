"""End-to-end validation of the surface-sampling depth-profile feature.

For each depth-profile type the test:
  1. runs remage with surface confinement and a depth profile on a large cube,
  2. reconstructs each vertex's depth (distance to the nearest cube face),
  3. asserts that every vertex lies inside the cube. The sampled depth is clamped
     to the distance to the far boundary, so no vertex can be generated outside
     the volume; this checks that guarantee end to end.
  4. compares the reconstructed depth distribution against the configured PDF with
     a binned Poisson likelihood-ratio (chi2) test and asserts significance < 5
     sigma. Only vertices near the centre of a face are used for this, so that
     edge/corner over-counting does not bias the depth spectrum.
  5. writes validation plots (depth histogram vs theory per profile, and a
     containment scatter).
"""

from __future__ import annotations

import lh5
import numpy as np
import pyg4ometry as pg4
from matplotlib import pyplot as plt
from remage import remage_run
from scipy import stats

plt.rcParams["lines.linewidth"] = 1
plt.rcParams["font.size"] = 12

GDML = "gdml/depth-box.gdml"
DET = "box"
HALF = 200.0  # mm, half-side of the cube defined in make_depth_box_gdml.py
NEVENTS = 200000
# a vertex is used for the depth spectrum only if its two in-plane distances to
# the box faces exceed this margin, i.e. it sits near the centre of one face and
# well away from any edge. Must comfortably exceed the largest sampled depth.
EDGE_MARGIN = 50.0  # mm

# per-profile configuration. "mean/sigma/lo/hi" are the macro substitution values
# (mm); parameters a given type ignores are set to valid dummy values so the
# messenger range checks pass. "slug" is used in the output plot file name.
PROFILES = {
    "Exponential": {
        "mean": 2.0,
        "sigma": 1.0,
        "lo": 0.0,
        "hi": 1.0,
        "slug": "exponential",
        "nice": "Exponential",
    },
    "Uniform": {
        "mean": 0.0,
        "sigma": 1.0,
        "lo": 0.5,
        "hi": 5.0,
        "slug": "uniform",
        "nice": "Uniform",
    },
    "TruncatedGaussian": {
        "mean": 3.0,
        "sigma": 1.0,
        "lo": 0.0,
        "hi": 6.0,
        "slug": "truncated-gaussian",
        "nice": "Truncated Gaussian",
    },
}


def theory(profile, cfg):
    """Return (pdf, cdf, dmax) for the configured profile (depth in mm)."""
    if profile == "Exponential":
        rv = stats.expon(scale=cfg["mean"])
        dmax = 6.0 * cfg["mean"]
    elif profile == "Uniform":
        rv = stats.uniform(loc=cfg["lo"], scale=cfg["hi"] - cfg["lo"])
        dmax = cfg["hi"] * 1.15
    elif profile == "TruncatedGaussian":
        lo, hi, mu, sg = cfg["lo"], cfg["hi"], cfg["mean"], cfg["sigma"]
        rv = stats.truncnorm((lo - mu) / sg, (hi - mu) / sg, loc=mu, scale=sg)
        dmax = cfg["hi"] * 1.15
    else:
        msg = f"unknown profile {profile}"
        raise ValueError(msg)
    return rv.pdf, rv.cdf, dmax


def run_profile(profile, cfg):
    outfile = f"test-depth-{cfg['slug']}-out.lh5"
    remage_run(
        "macros/gen-depth-profile.mac",
        gdml_files=GDML,
        macro_substitutions={
            "det": DET,
            "profile": profile,
            "mean": str(cfg["mean"]),
            "sigma": str(cfg["sigma"]),
            "lo": str(cfg["lo"]),
            "hi": str(cfg["hi"]),
            "nevents": str(NEVENTS),
        },
        overwrite_output=True,
        flat_output=True,
        log_level="summary",
        output=outfile,
    )
    return outfile


# box centre from the GDML (mirrors test_basic_surface.py)
reg = pg4.gdml.Reader(GDML).getRegistry()
center = np.array(reg.physicalVolumeDict[DET].position.eval())


def reconstruct(outfile):
    v = lh5.read_as("vtx", outfile, "ak")
    x = np.array(1000 * v.xloc.to_numpy()) - center[0]
    y = np.array(1000 * v.yloc.to_numpy()) - center[1]
    z = np.array(1000 * v.zloc.to_numpy()) - center[2]
    # per-axis distance to the nearest face; the smallest is the inward depth
    dists = np.sort(
        np.stack([HALF - np.abs(x), HALF - np.abs(y), HALF - np.abs(z)], axis=1)
    )
    depth = dists[:, 0]  # distance to the nearest face
    second = dists[:, 1]  # distance to the next-nearest face
    return x, y, z, depth, second


def fit_sigma(depth, cdf, dmax, nbins=50):
    counts, edges = np.histogram(depth, bins=nbins, range=(0, dmax))
    n = len(depth)
    # expected counts per bin from the exact CDF integral (handles partial bins)
    expected = n * (cdf(edges[1:]) - cdf(edges[:-1]))
    mask = expected > 10  # only well-populated bins
    obs = counts[mask].astype(float)
    exp = expected[mask]
    # Baker-Cousins Poisson likelihood-ratio statistic (~ chi2 with ndof)
    with np.errstate(divide="ignore", invalid="ignore"):
        term = np.where(obs > 0, obs * np.log(obs / exp), 0.0)
    test_stat = 2 * np.sum(exp - obs + term)
    ndof = int(mask.sum()) - 1
    p = stats.chi2.sf(test_stat, ndof)
    return stats.norm.ppf(1 - p) if p > 0 else np.inf


# run all profiles, check containment, keep results for plotting
results = {}
tol = 1e-6  # mm
for profile, cfg in PROFILES.items():
    outfile = run_profile(profile, cfg)
    x, y, z, depth, second = reconstruct(outfile)

    # containment: no vertex may be outside the cube (uses all vertices)
    outside = (
        (np.abs(x) > HALF + tol) | (np.abs(y) > HALF + tol) | (np.abs(z) > HALF + tol)
    )
    n_outside = int(np.sum(outside))
    assert n_outside == 0, f"{profile}: {n_outside} vertices generated outside the box"

    # depth spectrum from central-face vertices only (unbiased, edge-free)
    central = second > EDGE_MARGIN
    results[profile] = {
        "x": x,
        "z": z,
        "depth_central": depth[central],
        "n_outside": n_outside,
        "n_total": len(x),
        "cfg": cfg,
    }

# ---- per-profile depth histogram vs theory ----
for profile, res in results.items():
    cfg = res["cfg"]
    pdf, cdf, dmax = theory(profile, cfg)
    sigma = fit_sigma(res["depth_central"], cdf, dmax)

    fig, ax = plt.subplots(figsize=(7, 5))
    ax.hist(
        res["depth_central"],
        bins=50,
        range=(0, dmax),
        density=True,
        histtype="stepfilled",
        alpha=0.5,
        label="sampled vertices",
    )
    dd = np.linspace(0, dmax, 400)
    ax.plot(dd, pdf(dd), "r-", label="configured PDF")
    ax.set_xlabel("depth below surface [mm]")
    ax.set_ylabel("normalized density [1/mm]")
    ax.set_title(f"{cfg['nice']} depth profile ({sigma:.1f} $\\sigma$)")
    ax.legend()
    ax.grid()
    caption = (
        "Distribution of the inward depth of surface-sampled vertices (central-face "
        "vertices only) compared to the configured probability density."
    )
    plt.figtext(0.1, 0.02, caption, wrap=True, ha="left", fontsize=10)
    plt.tight_layout(rect=[0, 0.06, 1, 1])
    plt.savefig(f"depth-profile-{cfg['slug']}.output.png")
    plt.close(fig)

    assert sigma < 5, (
        f"{profile}: depth distribution deviates from theory at {sigma:.1f} sigma"
    )

# ---- containment scatter (exponential run) ----
res = results["Exponential"]
n_show = min(5000, res["n_total"])
fig, ax = plt.subplots(figsize=(6, 6))
ax.scatter(res["x"][:n_show], res["z"][:n_show], s=2, alpha=0.4)
ax.plot(
    [-HALF, HALF, HALF, -HALF, -HALF],
    [-HALF, -HALF, HALF, HALF, -HALF],
    "r-",
    label="box outline",
)
ax.set_xlabel("x position [mm]")
ax.set_ylabel("z position [mm]")
ax.set_aspect("equal")
ax.set_xlim(-1.1 * HALF, 1.1 * HALF)
ax.set_ylim(-1.1 * HALF, 1.1 * HALF)
ax.legend(loc="upper right")
ax.set_title(f"Containment: {res['n_outside']} / {res['n_total']} vertices outside")
caption = (
    "x-z projection of surface-sampled vertices with an exponential depth profile. "
    "All vertices lie inside the box outline: the depth clamp prevents any vertex "
    "from being generated outside the volume."
)
plt.figtext(0.05, 0.02, caption, wrap=True, ha="left", fontsize=10)
plt.tight_layout(rect=[0, 0.08, 1, 1])
plt.savefig("depth-profile-containment.output.png")
plt.close(fig)
