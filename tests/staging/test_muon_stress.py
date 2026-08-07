from __future__ import annotations

import json
import subprocess
import sys

import matplotlib.pyplot as plt


def _muon_macro(*, mode: str, backend: str, seed: int, events: int) -> str:
    """1 GeV muon fired through the liquid-argon world (OpticalPhysics on), keeping only events
    with a Germanium energy deposit. ``mode`` selects which secondaries are deferred, ``backend``
    how they are held during stage 0 -- the one thing the staging optimisation changes.

    ``mode``:
      * ``"optical"``  -> defer every optical photon.
      * ``"electron"`` -> defer every optical photon *and* every secondary electron/positron.
        Deferring the leptons keeps their sub-showers out of stage 0, so fewer photons pile up.

    ``backend`` (applied to every deferred species):
      * ``"g4"``    -> native Geant4 waiting stack: deferred tracks stay as full G4Tracks.
      * ``"ram"``   -> remage custom staging: each track recorded into a small POD struct
        (52 B per photon, 44 B per electron) kept in memory.
      * ``"spill"`` -> remage custom staging spilling records to a scratch file past
        LimitMemory, capping the in-memory footprint.

    The muon passes 0.5 m away from the Germanium, so every event is likely discarded:
    Re-injection correctness is covered separately by the optical round-trip test.
    """
    species = ["OpticalPhotons"]
    species_logic = ["/RMG/Staging/OpticalPhotons/DeferToWaitingStage true"]
    if mode == "electron":
        species.append("Electrons")
        species_logic += [
            "/RMG/Staging/Electrons/DeferToWaitingStage true",
            "/RMG/Staging/Electrons/IncludePositrons true",
            "/RMG/Staging/Electrons/VolumeSafety 1.0 cm",
            "/RMG/Staging/Electrons/MaxEnergyThresholdForStacking 10.0 MeV",
            "/RMG/Staging/Electrons/AddVolumeName world_vol",
        ]
    elif mode != "optical":
        msg = f"Unsupported mode: {mode}"
        raise ValueError(msg)

    backend_logic = []
    for s in species:
        if backend == "g4":
            backend_logic.append(f"/RMG/Staging/{s}/RMGDeferring false")
        elif backend == "ram":
            backend_logic.append(f"/RMG/Staging/{s}/RMGDeferring true")
        elif backend == "spill":
            limit_mb = 50 if s == "OpticalPhotons" else 1
            backend_logic += [
                f"/RMG/Staging/{s}/RMGDeferring true",
                f"/RMG/Staging/{s}/StorePath .",
                f"/RMG/Staging/{s}/LimitMemory {limit_mb}",
            ]
        else:
            msg = f"Unsupported backend: {backend}"
            raise ValueError(msg)

    staging_logic = "\n".join(species_logic + backend_logic)

    return f"""
/random/setSeeds {seed} {seed}
/RMG/Geometry/RegisterDetector Germanium detector_phys 0
/RMG/Output/ActivateOutputScheme Staging

/RMG/Processes/OpticalPhysics true

/run/initialize

/RMG/Output/Germanium/EdepCutLow 0 keV
{staging_logic}
/RMG/Output/Germanium/DiscardWaitingTracksUnlessGermaniumEdep true

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


def _grouped_bar(ax, backends, values, *, ylabel, title, log=False):
    """Two bars (optical / optical+e±) per backend."""
    width = 0.38
    x = list(range(len(backends)))
    ax.bar(
        [i - width / 2 for i in x],
        [values[("optical", b)] for b in backends],
        width,
        label="optical",
        color="tab:blue",
    )
    ax.bar(
        [i + width / 2 for i in x],
        [values[("electron", b)] for b in backends],
        width,
        label="optical + e±",
        color="tab:orange",
    )
    ax.set_xticks(x)
    ax.set_xticklabels(["G4 waiting stack", "Custom (RAM)", "Custom (spill)"])
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    if log:
        ax.set_yscale("log")
    ax.legend()
    ax.grid(ls=":", color="gray", alpha=0.5)


def test_muon_staging_backend_memory():
    """Peak RSS (and rate) of the staging backends on a heavy muon shower, for optical-photon
    staging alone and with electron/positron staging added, over the g4/ram/spill backends.

    The custom struct staging holds far less than full G4Tracks and spilling caps it further;
    additionally deferring the leptons keeps their sub-showers out of stage 0, which lowers the
    native waiting-stack peak too. The "electron" mode also exercises the RMGElectronState
    recording and scratch-file path under load."""
    events = 4
    modes = ["optical", "electron"]
    backends = ["g4", "ram", "spill"]
    seeds = {
        ("optical", "g4"): 601,
        ("optical", "ram"): 602,
        ("optical", "spill"): 603,
        ("electron", "g4"): 611,
        ("electron", "ram"): 612,
        ("electron", "spill"): 613,
    }

    metrics = {}
    for mode in modes:
        for backend in backends:
            output = f"muon-stress-{mode}-{backend}.lh5"
            macro = _muon_macro(
                mode=mode, backend=backend, seed=seeds[(mode, backend)], events=events
            )
            m = _run_in_subprocess(macro, output)
            m["rate_evt_s"] = events / m["elapsed_s"]
            metrics[(mode, backend)] = m

    rss_mb = {k: m["maxrss_kb"] / 1024 for k, m in metrics.items()}
    rate = {k: m["rate_evt_s"] for k, m in metrics.items()}

    fig, ax = plt.subplots()
    _grouped_bar(
        ax,
        backends,
        rss_mb,
        ylabel="Peak RSS (MB)",
        title="Muon Shower Peak Memory by Staging Backend",
        log=True,
    )
    fig.savefig("muon_stress_backend_memory.png", dpi=300, bbox_inches="tight")
    fig.clf()

    fig, ax = plt.subplots()
    _grouped_bar(
        ax,
        backends,
        rate,
        ylabel="Processing Rate (events/s)",
        title="Muon Shower Processing Rate by Staging Backend",
    )
    fig.savefig("muon_stress_backend_rate.png", dpi=300, bbox_inches="tight")
    fig.clf()

    def rss(mode, backend):
        return metrics[(mode, backend)]["maxrss_kb"]

    # In both modes: the small structs hold far less than full G4Tracks, and spilling caps the
    # in-memory footprint below keeping every struct in RAM.
    for mode in modes:
        assert rss(mode, "ram") < rss(mode, "g4"), (
            f"{mode}: custom RAM staging did not reduce peak RSS: "
            f"ram={rss(mode, 'ram')} KB, g4={rss(mode, 'g4')} KB"
        )
        assert rss(mode, "spill") < rss(mode, "ram"), (
            f"{mode}: spilling did not reduce peak RSS below RAM: "
            f"spill={rss(mode, 'spill')} KB, ram={rss(mode, 'ram')} KB"
        )

    # Deferring the leptons keeps their sub-showers out of stage 0, lowering the native
    # waiting-stack peak compared with staging only the optical photons.
    assert rss("electron", "g4") < rss("optical", "g4"), (
        f"electron staging did not lower the waiting-stack peak: "
        f"electron={rss('electron', 'g4')} KB, optical={rss('optical', 'g4')} KB"
    )
