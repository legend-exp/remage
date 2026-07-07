from __future__ import annotations

import lh5
from remage import remage_run


def run_and_count(macro: str, output_lh5: str):
    """Run ``macro`` through remage and return the per-particle track counts.

    Produces flat lh5 output at ``output_lh5`` and returns the ``value_counts``
    of the ``particle`` column (a pandas Series indexed by PDG code).
    """
    remage_run(
        macro,
        gdml_files="gdml/geometry.gdml",
        output=output_lh5,
        flat_output=True,
        overwrite_output=True,
    )

    tracks = lh5.read("tracks", output_lh5).view_as("pd")
    return tracks["particle"].value_counts()
