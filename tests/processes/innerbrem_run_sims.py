from __future__ import annotations

from remage import remage_run

remage_run(
    macros="macros/run_IB.mac",
    gdml_files="gdml/geometry.gdml",
    output="output_IB.lh5",
    overwrite_output=True,
)

remage_run(
    macros="macros/run_noIB.mac",
    gdml_files="gdml/geometry.gdml",
    output="output_noIB.lh5",
    overwrite_output=True,
)

remage_run(
    macros="macros/run_IB_track.mac",
    gdml_files="gdml/geometry.gdml",
    output="track.lh5",
    overwrite_output=True,
)
