from __future__ import annotations

from remage import remage_run

modes = ["true", "false"]
energies = ["500", "2000", "5000"]  # in keV

# Run the other three
for mode in modes:
    for energy in energies:
        remage_run(
            macros="macros/template.mac",
            gdml_files="gdml/geometry-box.gdml",
            output=f"{'combined' if mode == 'true' else 'uncombined'}_{energy}.lh5",
            overwrite_output=True,
            macro_substitutions={
                "MODE": mode,
                "ENERGY": energy,
            },
        )
