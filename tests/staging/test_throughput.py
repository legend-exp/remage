from __future__ import annotations

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
    events = scaled_events(200)
    seeds = [501, 502]

    rates = {"optical_stacking": [], "electron_stacking": []}

    for mode in ("optical_stacking", "electron_stacking"):
        for seed in seeds:
            output = f"throughput-{mode}-{seed}.lh5"
            macro = _throughput_macro(
                mode=mode,
                seed=seed,
                events=events,
                distance_cm=100.0,
            )
            elapsed = run_macro(macro, output, log_level="summary")
            rates[mode].append(events / elapsed)

    optical_rate = float(np.median(rates["optical_stacking"]))
    electron_rate = float(np.median(rates["electron_stacking"]))

    assert electron_rate > optical_rate, (
        f"Expected electron stacking to be faster. "
        f"optical={optical_rate:.3f} evt/s, electron={electron_rate:.3f} evt/s"
    )
