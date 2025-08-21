from __future__ import annotations

from remage import remage_run

runs = ["2vbb", "0vbb_M1", "0vbb_lambda_0"]

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

# Run the other three
for run_name in runs:
    remage_run(
        macros="macros/template.mac",
        gdml_files="gdml/geometry.gdml",
        output=f"{run_name}.lh5",
        overwrite_output=True,
        macro_substitutions={
            "MODE": run_name,
        },
    )
