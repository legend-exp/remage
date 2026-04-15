from __future__ import annotations

import multiprocessing as mp
from concurrent.futures import ProcessPoolExecutor, as_completed

import matplotlib as mpl
import matplotlib.pyplot as plt
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
    distance: float,
) -> str:
    staging_activation = "/RMG/Output/ActivateOutputScheme Staging"
    if mode == "optical_stacking":
        electron_staging_logic = ""
    elif mode == "electron_stacking":
        electron_staging_logic = "\n".join(
            [
                "/RMG/Staging/Electrons/DeferToWaitingStage true",
                f"/RMG/Staging/Electrons/VolumeSafety {safety_cm} cm",
                f"/RMG/Staging/Electrons/MaxEnergyThresholdForStacking {threshold_mev} MeV",
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

/run/initialize

/RMG/Output/Germanium/EdepCutLow 10 keV
/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Output/Germanium/DiscardWaitingTracksUnlessGermaniumEdep true
{electron_staging_logic}

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position 0 0 {distance} cm
/gps/particle gamma
/gps/energy 2.6 MeV
/gps/ang/type iso

/run/beamOn {events}
"""


def _low_energy_chi2(reference: np.ndarray, candidate: np.ndarray) -> float:
    bins = np.linspace(0.0, 500.0, 51)
    ref = reference[(reference >= 0.0) & (reference < 500.0)]
    cand = candidate[(candidate >= 0.0) & (candidate < 500.0)]

    if ref.size == 0 or cand.size == 0:
        return float("inf")

    ref_hist, _ = np.histogram(ref, bins=bins)
    cand_hist, _ = np.histogram(cand, bins=bins)

    # Add a small constant to avoid division by zero
    epsilon = 1e-6
    return np.sum((ref_hist - cand_hist) ** 2 / (ref_hist + epsilon))


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


def _run_macro_job(job: tuple[str, str]) -> str:
    output, macro = job
    run_macro(macro, output, log_level="summary", threads=10)
    return output


def _run_macros_in_parallel(jobs: list[tuple[str, str]]) -> None:
    if not jobs:
        return

    # Use spawn to avoid inheriting fragile state from the parent test process.
    context = mp.get_context("spawn")
    max_workers = (
        int(min(len(jobs), (mp.cpu_count() / 2) // 10)) if mp.cpu_count() > 1 else 1
    )
    with ProcessPoolExecutor(max_workers=max_workers, mp_context=context) as executor:
        futures = {executor.submit(_run_macro_job, job): job[0] for job in jobs}
        for future in as_completed(futures):
            output = futures[future]
            try:
                future.result()
            except Exception as exc:
                msg = f"Parallel macro execution failed for output '{output}': {exc}"
                raise RuntimeError(msg) from exc


def test_energy_consistency_is_safety_independent():
    """Test that the energy spectrum of events recorded in the germanium detector is consistent between optical stacking and electron stacking."""
    events = scaled_events(100000)
    threshold_mev = 10.0
    safety_scan_cm = [1.0, 3.1, 10.0, 31]
    distances = np.array([10, 21, 46])

    baseline_output = "physics-regression-optical-baseline-{d}.lh5"
    baseline_jobs = [
        (
            baseline_output.format(d=d),
            _physics_macro(
                mode="optical_stacking",
                seed=701,
                events=int(events * (d / distances[0])),
                safety_cm=0.0,
                threshold_mev=threshold_mev,
                distance=d,
            ),
        )
        for d in distances
    ]
    _run_macros_in_parallel(baseline_jobs)

    baseline_steps = {
        d: read_detector_steps(baseline_output.format(d=d)) for d in distances
    }
    baseline_spec = {d: event_energy_spectrum(baseline_steps[d]) for d in distances}

    electron_outputs = {
        (d, safety_cm): f"physics-regression-electron-{d}-safety-{safety_cm}.lh5"
        for safety_cm in safety_scan_cm
        for d in distances
    }
    electron_jobs = [
        (
            electron_outputs[(d, safety_cm)],
            _physics_macro(
                mode="electron_stacking",
                seed=710 + idx,
                events=int(events * (d / distances[0])),
                safety_cm=safety_cm,
                threshold_mev=threshold_mev,
                distance=d,
            ),
        )
        for idx, safety_cm in enumerate(safety_scan_cm)
        for d in distances
    ]
    _run_macros_in_parallel(electron_jobs)

    low_diffs = {}
    annihilation_diffs = {}
    electron_spec = {}
    for safety_cm in safety_scan_cm:
        for d in distances:
            output = electron_outputs[(d, safety_cm)]

            steps = read_detector_steps(output)
            spec = event_energy_spectrum(steps)
            electron_spec[(d, safety_cm)] = spec

            low_diff = _low_energy_chi2(baseline_spec[d], spec)
            if d not in low_diffs:
                low_diffs[d] = []
            low_diffs[d].append(low_diff)

            annihilation_diff = _positron_annihilation_ratio(baseline_spec[d], spec)
            if d not in annihilation_diffs:
                annihilation_diffs[d] = []
            annihilation_diffs[d].append(annihilation_diff)

            high_ratio = _high_energy_count_ratio(baseline_spec[d], spec)
            if len(spec) > 20:
                assert 0.8 <= high_ratio <= 1.2, (
                    f"High-energy inconsistency at safety={safety_cm} cm: "
                    f"distance={d} cm, "
                    f"ratio={high_ratio:.3f}"
                    f"spectrum={spec}"
                )

    # plot total energy soectra and residuals between baseline and each safety scan
    for d in distances:
        fig, ax = plt.subplots(
            2,
            1,
            sharex=True,
            figsize=(5, 7),
            gridspec_kw={"height_ratios": [5, 1]},
        )
        bins = np.linspace(0.0, 3000.0, 31)
        ax[0].hist(
            baseline_spec[d],
            bins=bins,
            histtype="step",
            label="Optical Stacking Baseline",
            color="black",
        )
        for i, safety_cm in enumerate(safety_scan_cm):
            spec = electron_spec[(d, safety_cm)]
            ax[0].hist(
                spec,
                bins=bins,
                histtype="step",
                label=f"Electron Stacking Safety {safety_cm} cm",
                color=mpl.colormaps["tab10"](i),
            )
        ax[0].set_ylabel("Counts")
        ax[0].set_title(f"Energy Spectrum Comparison at Distance {d} cm")
        ax[0].legend()
        ax[0].grid()

        for i, safety_cm in enumerate(safety_scan_cm):
            spec = electron_spec[(d, safety_cm)]
            residual = (
                np.histogram(spec, bins=bins)[0]
                - np.histogram(baseline_spec[d], bins=bins)[0]
            ) / np.sqrt(np.histogram(baseline_spec[d], bins=bins)[0] + 1e-6)
            ax[1].plot(
                (bins[:-1] + bins[1:]) / 2,
                residual,
                label=f"Residual Safety {safety_cm} cm",
                lw=0,
                marker=".",
                color=mpl.colormaps["tab10"](i),
            )
        ax[1].set_xlabel("Energy (keV)")
        ax[1].set_ylabel("Residual")
        ax[1].grid()

        fig.savefig(
            f"energy_spectrum_comparison_{d}cm.png", dpi=300, bbox_inches="tight"
        )
        fig.clf()

    for values in low_diffs.values():
        for i in range(len(values)):
            assert values[i] < 200, (
                "Low-energy mismatch is larger than expected for safety-independent behavior: "
                f"{values}"
            )
