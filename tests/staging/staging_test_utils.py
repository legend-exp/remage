from __future__ import annotations

import os
import time

import awkward as ak
import numpy as np
from lgdo import lh5
from remage import remage_run


def scaled_events(base_events: int) -> int:
    factor = int(os.environ.get("RMG_STATS_FACTOR", "1"))
    return max(1, base_events * max(1, factor))


def run_macro(
    macro: str,
    output: str,
    *,
    gdml_file: str = "gdml/geometry.gdml",
    log_level: str = "error",
) -> float:
    start = time.perf_counter()
    remage_run(
        macro.splitlines(),
        gdml_files=gdml_file,
        output=output,
        overwrite_output=True,
        log_level=log_level,
    )
    return time.perf_counter() - start


def read_tracks(output: str):
    try:
        return lh5.read_as("tracks", output, library="ak")
    except Exception:
        return None


def read_detector_steps(output: str, detector: str = "detector_phys"):
    try:
        return lh5.read_as(f"stp/{detector}", output, library="ak")
    except Exception:
        return None


def count_tracks_by_stage(
    tracks,
    pdg: int,
    stage: int,
    primary_only: bool | None = None,
) -> int:
    if tracks is None or len(tracks) == 0:
        return 0
    if "stageid" not in tracks.fields:
        return 0

    mask = tracks.particle == pdg
    if primary_only is True:
        mask = mask & (tracks.parent_trackid == 0)
    elif primary_only is False:
        mask = mask & (tracks.parent_trackid > 0)

    mask = mask & (tracks.stageid == stage)
    return int(ak.sum(mask))


def triggered_event_count(step_data) -> int:
    if step_data is None or len(step_data) == 0:
        return 0
    return len(np.unique(ak.to_numpy(step_data.evtid)))


def event_energy_spectrum(step_data) -> np.ndarray:
    if step_data is None or len(step_data) == 0:
        return np.array([], dtype=float)

    # grouped = ak.unflatten(step_data.edep, ak.run_lengths(step_data.evtid))
    return ak.to_numpy(ak.ravel(ak.sum(step_data.edep, axis=-1)))
