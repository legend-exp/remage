from __future__ import annotations

import matplotlib.pyplot as plt
import numpy as np
from staging_test_utils import run_macro, scaled_events


def _throughput_macro(*, mode: str, seed: int, events: int, distance_cm: float) -> str:
    staging_activation = "/RMG/Output/ActivateOutputScheme Staging"
    if mode == "optical_stacking":
        electron_staging_logic = ""
    elif mode == "electron_stacking":
        electron_staging_logic = "\n".join(
            [
                "/RMG/Staging/Electrons/DeferToWaitingStage true",
                "/RMG/Staging/Electrons/VolumeSafety 20.0 cm",
                "/RMG/Staging/Electrons/MaxEnergyThresholdForStacking 3.0 MeV",
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

/RMG/Output/Germanium/EdepCutLow 10 keV
/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Output/Germanium/DiscardWaitingTracksUnlessGermaniumEdep true
{electron_staging_logic}

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position {distance_cm} 0 0 cm
/gps/particle gamma
/gps/ang/type iso
/gps/energy 2.6 MeV

/run/beamOn {events}
"""


def test_gamma_optical_event_rate_electron_stacking_is_faster():
    """Test that the event processing rate is faster with electron stacking than with optical stacking for gamma events."""
    events = scaled_events(100)
    seed = 501
    distances = [10, 30, 100]

    rates = {"optical_stacking": [], "electron_stacking": []}

    for mode in ("optical_stacking", "electron_stacking"):
        for d in distances:
            output = f"throughput-{mode}-{d}.lh5"
            macro = _throughput_macro(
                mode=mode,
                seed=seed,
                events=events,
                distance_cm=d,
            )
            elapsed = run_macro(macro, output, log_level="summary")
            rates[mode].append(events / elapsed)

    # plot event rate for each mode
    fig, ax = plt.subplots(figsize=(5, 5))
    for mode in ("optical_stacking", "electron_stacking"):
        ax.plot(
            distances,
            1 / np.array(rates[mode]),
            marker="x",
            label=mode.replace("_", " ").title(),
        )
    ax.set_xlabel("Mode")
    ax.set_ylabel("Time per event (sec/evt)")
    ax.set_title("Simulation Time for Different Simulation Modes and Distances")
    ax.set_yscale("log")
    ax.legend(title="Distance from Source:")
    ax.grid(ls=":", color="gray", alpha=0.5)
    fig.savefig("throughput_distance_scan.png", dpi=300, bbox_inches="tight")

    for optical_rate, electron_rate, d in zip(
        rates["optical_stacking"], rates["electron_stacking"], distances, strict=True
    ):
        assert electron_rate > optical_rate, (
            f"Expected electron stacking to be faster. "
            f"optical={optical_rate:.3f} evt/s, electron={electron_rate:.3f} evt/s, distance={d} cm"
        )
