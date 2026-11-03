from __future__ import annotations

from remage import remage_run

# output file name -> arguments of /RMG/Generator/BxDecay0/DoubleBetaDecay Ge76 (process and level
# of the Se76 daughter nucleus)
runs = {
    "2vbb": "2vbb",
    "0vbb_M1": "0vbb_M1",
    "0vbb_M2": "0vbb_M2",
    "0vbb_M3": "0vbb_M3",
    "0vbb_M7": "0vbb_M7",
    "0vbb_lambda_0": "0vbb_lambda_0",
    # transitions to the excited states of Se76: 2+1 (559 keV, level 1), 0+1 (1122 keV, level 2)
    # and 2+2 (1216 keV, level 3)
    "2vbb_2p1": "2vbb_2 1",
    "2vbb_0p1": "2vbb 2",
    "2vbb_2p2": "2vbb_2 3",
    "0vbb_lambda_2p1": "0vbb_lambda_2 1",
}

# Run the template 2 once to test the different order of vertex gen macros
remage_run(
    macros="macros/template2.mac",
    gdml_files="gdml/geometry.gdml",
    output="0vbb.lh5",
    overwrite_output=True,
    macro_substitutions={
        "MODE": "0vbb",
    },
)

for run_name, mode in runs.items():
    remage_run(
        macros="macros/template.mac",
        gdml_files="gdml/geometry.gdml",
        output=f"{run_name}.lh5",
        overwrite_output=True,
        macro_substitutions={
            "MODE": mode,
        },
    )
