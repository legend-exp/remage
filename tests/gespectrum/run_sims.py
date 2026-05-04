from __future__ import annotations

import os

from remage import remage_run

stats_factor = int(os.environ["RMG_STATS_FACTOR"])
n_events = 50000 * 4 ** min(stats_factor, 5)

remage_run(
    macros="macros/thsource-full.mac",
    macro_substitutions={
        "EVENTS": n_events,
    },
    gdml_files="gdml/hades-test.gdml",
    output="hades-sim.lh5",
    overwrite_output=True,
    merge_output_files=True,
    log_level="summary",
    threads=max(32, os.cpu_count()),
)
