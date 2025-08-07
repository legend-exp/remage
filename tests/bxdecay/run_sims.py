from __future__ import annotations

from remage import remage_run

bbv0_commands = [
    "macros/template.mac",
    "/RMG/Generator/BxDecay0/DoubleBetaDecay Ge76 0vbb",
    "/run/beamOn 1000000",
]
# the two template macros are just to incorporate a test that makes sure the order of vertex macros does not matter
bbv2_commands = [
    "macros/template2.mac",
    "/RMG/Generator/BxDecay0/DoubleBetaDecay Ge76 2vbb",
    "/run/beamOn 1000000",
]

runs = [
    ("0vbb", bbv0_commands),
    ("2vbb", bbv2_commands),
]

# Run each simulation
for run_name, cmds in runs:
    remage_run(
        cmds,
        gdml_files="gdml/geometry.gdml",
        output=f"{run_name}.lh5",
        overwrite_output=True,
    )
