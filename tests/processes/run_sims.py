from __future__ import annotations

from remage import remage_run

# Run the template 2 once to test the different order of vertex gen macros
remage_run(
    macros="macros/run_IB.mac",
    gdml_files="gdml/geometry.gdml",
    output="output_IB.lh5",
    overwrite_output=True
)


remage_run(
    macros="macros/run_noIB.mac",
    gdml_files="gdml/geometry.gdml",
    output=f"output_noIB.lh5",
    overwrite_output=True
)

remage_run(
    macros="macros/run_track.mac",
    gdml_files="gdml/geometry.gdml",
    output=f"track.lh5",
    overwrite_output=True
)
