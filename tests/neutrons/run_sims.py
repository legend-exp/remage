from __future__ import annotations

from remage import remage_run

remage_run(
    macros="macros/template.mac",
    gdml_files="gdml/geometry.gdml",
    output="neutron_captures_evap.lh5",
    overwrite_output=True,
    macro_substitutions={
        "EVAP": "true",
    },
)


remage_run(
    macros="macros/template.mac",
    gdml_files="gdml/geometry.gdml",
    output="neutron_captures_normal.lh5",
    overwrite_output=True,
    macro_substitutions={
        "EVAP": "false",
    },
)
