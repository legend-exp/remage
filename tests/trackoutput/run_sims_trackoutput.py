from __future__ import annotations

from remage import remage_run_from_args

base_args = [
    "--macro-substitutions",
    "STORE={}",
    "--gdml-files",
    "gdml/geometry.gdml",
    "--output-file",
    "track-store-{}.lh5",
    "--overwrite",
    "--quiet",
    "macros/run.mac",
]

args_store = [arg.format("true") for arg in base_args]
args_discard = [arg.format("false") for arg in base_args]

remage_run_from_args(args_store, raise_on_error=True)
remage_run_from_args(args_discard, raise_on_error=True)
