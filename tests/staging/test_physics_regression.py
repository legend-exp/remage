from __future__ import annotations

import numpy as np
from staging_test_utils import (
    event_energy_spectrum,
    read_detector_steps,
    run_macro,
    scaled_events,
)


def _physics_macro(
    *,
    mode: str,
    seed: int,
    events: int,
    safety_cm: float,
    threshold_mev: float,
) -> str:
    if mode == "optical_stacking":
        stacker_activation = ""
        stacker_logic = ""
    elif mode == "electron_stacking":
        stacker_activation = "/RMG/Output/ActivateOutputScheme VolumeStacker"
        stacker_logic = "\n".join(
            [
                f"/RMG/Output/VolumeStacker/VolumeSafety {safety_cm} cm",
                f"/RMG/Output/VolumeStacker/MaxEnergyThresholdForStacking {threshold_mev} MeV",
                "/RMG/Output/VolumeStacker/AddVolumeName world_vol",
            ]
        )
    else:
        msg = f"Unsupported mode: {mode}"
        raise ValueError(msg)

    return f"""
/random/setSeeds {seed} {seed}
/RMG/Geometry/RegisterDetector Germanium detector_phys 0
{stacker_activation}

/run/initialize

/RMG/Output/Germanium/EdepCutLow 10 keV
/RMG/Output/Germanium/DiscardPhotonsIfNoGermaniumEdep true
{stacker_logic}

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position 0 0 5 cm
/gps/particle gamma
/gps/energy 2.6 MeV
/gps/ang/type iso

/run/beamOn {events}
"""


def _low_energy_difference(reference: np.ndarray, candidate: np.ndarray) -> float:
    bins = np.linspace(0.0, 500.0, 51)
    ref = reference[(reference >= 0.0) & (reference < 500.0)]
    cand = candidate[(candidate >= 0.0) & (candidate < 500.0)]

    if ref.size == 0 or cand.size == 0:
        return float("inf")

    ref_hist, _ = np.histogram(ref, bins=bins)
    cand_hist, _ = np.histogram(cand, bins=bins)

    ref_norm = ref_hist / ref_hist.sum()
    cand_norm = cand_hist / cand_hist.sum()
    return float(np.sum(np.abs(cand_norm - ref_norm)))


def _high_energy_count_ratio(reference: np.ndarray, candidate: np.ndarray) -> float:
    emin, emax = 1500.0, 3000.0
    ref_count = int(np.count_nonzero((reference >= emin) & (reference < emax)))
    cand_count = int(np.count_nonzero((candidate >= emin) & (candidate < emax)))

    if ref_count == 0:
        return 0.0
    return cand_count / ref_count


def _positron_annihilation_ratio(reference: np.ndarray, candidate: np.ndarray) -> float:
    peak_energy = 511.0
    window = 2.5
    ref_count = int(
        np.count_nonzero(
            (reference >= peak_energy - window) & (reference < peak_energy + window)
        )
    )
    cand_count = int(
        np.count_nonzero(
            (candidate >= peak_energy - window) & (candidate < peak_energy + window)
        )
    )

    if ref_count == 0:
        return 0.0
    return cand_count / ref_count


def test_energy_consistency_is_safety_independent():
    """Test that the energy spectrum of events recorded in the germanium detector is consistent between optical stacking and electron stacking."""
    events = scaled_events(50000)
    threshold_mev = 10.0
    safety_scan_cm = [1.0, 5.0, 20.0]

    baseline_output = "physics-regression-optical-baseline.lh5"
    baseline_macro = _physics_macro(
        mode="optical_stacking",
        seed=701,
        events=events,
        safety_cm=0.0,
        threshold_mev=threshold_mev,
    )
    run_macro(baseline_macro, baseline_output, log_level="summary")

    baseline_steps = read_detector_steps(baseline_output)
    baseline_spec = event_energy_spectrum(baseline_steps)
    assert baseline_spec.size > 100, "Not enough baseline events for regression checks"

    low_diffs = []
    annihilation_diffs = []
    for idx, safety_cm in enumerate(safety_scan_cm):
        output = f"physics-regression-electron-safety-{safety_cm}.lh5"
        macro = _physics_macro(
            mode="electron_stacking",
            seed=710 + idx,
            events=events,
            safety_cm=safety_cm,
            threshold_mev=threshold_mev,
        )
        run_macro(macro, output, log_level="summary")

        steps = read_detector_steps(output)
        spec = event_energy_spectrum(steps)
        assert spec.size > 50, f"Not enough events for safety={safety_cm} cm"

        low_diff = _low_energy_difference(baseline_spec, spec)
        low_diffs.append(low_diff)

        annihilation_diff = _positron_annihilation_ratio(baseline_spec, spec)
        annihilation_diffs.append(annihilation_diff)

        high_ratio = _high_energy_count_ratio(baseline_spec, spec)
        assert 0.8 <= high_ratio <= 1.2, (
            f"High-energy inconsistency at safety={safety_cm} cm: "
            f"ratio={high_ratio:.3f}"
        )

    for low_diff in low_diffs:
        assert low_diff <= 0.30, (
            "Low-energy mismatch is larger than expected for safety-independent behavior: "
            f"{low_diffs}"
        )

    assert max(low_diffs) - min(low_diffs) <= 0.03, (
        "Low-energy mismatch depends too strongly on safety, contrary to observed behavior: "
        f"{low_diffs}"
    )

    for ann_diff in annihilation_diffs:
        assert 0.6 <= ann_diff <= 1.0, (
            "Positron annihilation is expected to be lower, but here it is significantly different: "
            f"{annihilation_diffs}"
        )
