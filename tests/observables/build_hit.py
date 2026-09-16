"""Post-processing of the germanium scans, shared by the observables and clustering tests.

The post-processing is a plain python script calling the reboost processors on
the tables of the remage output.
"""

from __future__ import annotations

from pathlib import Path

import awkward as ak
import lh5
import numpy as np
import reboost
from lgdo import Array, Table

# parameters of the piecewise-linear HPGe activeness model
FCCD_IN_MM = 1
DEAD_LAYER_FRACTION = 0.5

# shifts the germanium z coordinate to the bottom face of the detector
Z_OFFSET_IN_MM = 20

# the step position variants remage writes with `StepPositionMode Both`: the
# average of the two step points (no suffix) and the two step points themselves
POSITION_VARIANTS = {"avg": "", "pre": "_pre", "post": "_post"}


def build_hit(stp_file: str | Path, hit_file: str | Path) -> None:
    """Reconstruct the germanium observables and the vertices of a step file.

    Writes the ``hit/germanium`` and ``hit/vtx`` tables read by
    ``plot_observables.py`` to `hit_file`, overwriting it.
    """
    stp_file = str(stp_file)
    hit_file = str(hit_file)

    _build_germanium_hits(stp_file, hit_file)
    _build_vertices(stp_file, hit_file)


def _build_germanium_hits(stp_file: str, hit_file: str) -> None:
    stps = lh5.read("stp/germanium", stp_file)
    steps = stps.view_as("ak", with_units=True)

    # carries over the evtid and t0 fields, i.e. what a TCM needs
    hits = reboost.init_hit_table(stps)

    edep = reboost.units.units_conv_ak(steps.edep, "keV")
    hits.add_field("truth_energy", _energy(ak.sum(edep, axis=-1)))

    for variant, suffix in POSITION_VARIANTS.items():
        xloc, yloc, zloc = (steps[f"{c}loc{suffix}"] for c in ("x", "y", "z"))

        # weight the energy of every step with the charge collection efficiency
        # at its distance from the detector surface
        activeness = reboost.math.piecewise_linear_activeness(
            steps[f"dist_to_surf{suffix}"],
            fccd_in_mm=FCCD_IN_MM,
            dlf=DEAD_LAYER_FRACTION,
        )
        hits.add_field(
            f"active_energy_{variant}", _energy(ak.sum(edep * activeness, axis=-1))
        )

        # radius of the sphere holding 90% of the energy of the hit
        hits.add_field(
            f"r90_{variant}", _length(reboost.hpge.r90(edep, xloc, yloc, zloc))
        )

        # highest point of the hit, measured from the bottom of the detector
        max_z = ak.max(reboost.units.units_conv_ak(zloc, "mm"), axis=-1)
        hits.add_field(
            f"max_z_{variant}", _length(ak.fill_none(max_z, np.nan) + Z_OFFSET_IN_MM)
        )

    lh5.write(hits, "hit/germanium", hit_file, wo_mode="overwrite_file")


def _build_vertices(stp_file: str, hit_file: str) -> None:
    vtx = lh5.read("vtx", stp_file)
    verts = vtx.view_as("ak", with_units=True)

    xloc, yloc = (
        reboost.units.units_conv_ak(verts[f"{c}loc"], "m") for c in ("x", "y")
    )

    out = Table(size=len(vtx))
    for field in ("evtid", "xloc", "yloc", "zloc"):
        out.add_field(field, vtx[field])
    out.add_field(
        "rloc", Array(ak.to_numpy(np.sqrt(xloc**2 + yloc**2)), attrs={"units": "m"})
    )

    lh5.write(out, "hit/vtx", hit_file, wo_mode="append")


def _energy(data: ak.Array) -> Array:
    return Array(ak.to_numpy(data), attrs={"units": "keV"})


def _length(data: ak.Array) -> Array:
    return Array(ak.to_numpy(data), attrs={"units": "mm"})
