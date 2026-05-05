from __future__ import annotations

import json
import subprocess
import sys

import matplotlib.pyplot as plt


def _muon_macro(*, mode: str, seed: int, events: int) -> str:
    staging_activation = "/RMG/Output/ActivateOutputScheme Staging"
    if mode == "optical_stacking":
        electron_staging_logic = ""
    elif mode == "electron_stacking":
        electron_staging_logic = "\n".join(
            [
                "/RMG/Staging/Electrons/DeferToWaitingStage true",
                "/RMG/Staging/Electrons/VolumeSafety 1.0 cm",
                "/RMG/Staging/Electrons/MaxEnergyThresholdForStacking 10.0 MeV",
                "/RMG/Staging/Electrons/AddVolumeName world_vol",
            ]
        )
    else:
        msg = f"Unsupported mode: {mode}"
        raise ValueError(msg)

    return f"""
/random/setSeeds {seed} {seed}
/RMG/Geometry/RegisterDetector Germanium detector_phys 0
{staging_activation}

/RMG/Processes/OpticalPhysics true

/run/initialize

/RMG/Output/Germanium/EdepCutLow 0 keV
/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Output/Germanium/DiscardWaitingTracksUnlessGermaniumEdep true
{electron_staging_logic}

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position 0.5 0 1 m
/gps/particle mu-
/gps/energy 1 GeV
/gps/direction 0 0 -1

/run/beamOn {events}
"""


def _run_in_subprocess(macro: str, output: str) -> dict:
    payload = json.dumps({"macro": macro, "output": output})
    child_code = (
        f"""
import json
import resource
import time
from remage import remage_run

payload = json.loads({payload!r})
start = time.perf_counter()
remage_run(
    payload[\"macro\"].splitlines(),
    gdml_files=\"gdml/geometry.gdml\",
    output=payload[\"output\"],
    overwrite_output=True,
    log_level=\"summary\"
)
elapsed = time.perf_counter() - start

# remage_run executes remage-cpp in child process(es).
rss_self_kb = int(resource.getrusage(resource.RUSAGE_SELF).ru_maxrss)
rss_children_kb = int(resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss)
rss_sim_kb = rss_children_kb if rss_children_kb > 0 else rss_self_kb
"""
        + """
print(json.dumps({
    \"elapsed_s\": elapsed,
    \"maxrss_kb\": rss_sim_kb,
    \"maxrss_self_kb\": rss_self_kb,
    \"maxrss_children_kb\": rss_children_kb,
}))
"""
    )

    proc = subprocess.run(
        [sys.executable, "-c", child_code],
        check=True,
        capture_output=True,
        text=True,
    )

    print(f"Subprocess output:\n{proc.stdout}\n{proc.stderr}", file=sys.stderr)

    for line in reversed((proc.stdout + "\n" + proc.stderr).splitlines()):
        line_striped = line.strip()
        if line_striped.startswith("{") and line_striped.endswith("}"):
            return json.loads(line_striped)

    msg = "Could not parse child-process metrics output"
    raise AssertionError(msg)


def test_muon_memory_and_rate_directionality():
    """Test that electron stacking reduces the peak RSS for muon events compared to optical stacking, while improving the processing rate."""
    events = 4

    metrics = {}
    for mode, seed in (("optical_stacking", 601), ("electron_stacking", 602)):
        output = f"muon-stress-{mode}.lh5"
        macro = _muon_macro(mode=mode, seed=seed, events=events)
        mode_metrics = _run_in_subprocess(macro, output)
        mode_metrics["rate_evt_s"] = events / mode_metrics["elapsed_s"]
        metrics[mode] = mode_metrics

    optical = metrics["optical_stacking"]
    electron = metrics["electron_stacking"]

    # memory consumption example
    fig, ax = plt.subplots()
    ax.bar(
        ["Optical Stacking", "Electron Stacking"],
        [optical["maxrss_kb"] / 1024, electron["maxrss_kb"] / 1024],
        color=["tab:blue", "tab:orange"],
    )
    ax.set_ylabel("Peak RSS (GB)")
    ax.set_title("Muon Event Peak Memory Usage")
    ax.grid(ls=":", color="gray", alpha=0.5)
    fig.savefig("muon_stress_memory.png", dpi=300, bbox_inches="tight")
    fig.clf()

    # time per event example
    fig, ax = plt.subplots()
    ax.bar(
        ["Optical Stacking", "Electron Stacking"],
        [optical["rate_evt_s"], electron["rate_evt_s"]],
        color=["tab:blue", "tab:orange"],
    )
    ax.set_ylabel("Processing Rate (events/s)")
    ax.set_title("Muon Event Processing Rate")
    ax.grid(ls=":", color="gray", alpha=0.5)
    fig.savefig("muon_stress_rate.png", dpi=300, bbox_inches="tight")
    fig.clf()

    assert electron["maxrss_kb"] < optical["maxrss_kb"], (
        f"Electron stacking did not reduce muon peak RSS enough: "
        f"optical={optical['maxrss_kb']} KB, electron={electron['maxrss_kb']} KB, "
        f"optical_children={optical.get('maxrss_children_kb')} KB, "
        f"electron_children={electron.get('maxrss_children_kb')} KB"
    )
