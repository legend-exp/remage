from __future__ import annotations

from staging_test_utils import (
    count_duplicate_trackids,
    count_tracks_by_stage,
    read_tracks,
    run_macro,
    scaled_events,
)


def _primary_macro(
    *,
    seed: int,
    particle: str,
    energy_mev: float,
    safety_cm: float,
    threshold_mev: float,
    z_cm: float,
    events: int,
    stack_volume: str = "world_vol",
) -> str:
    return f"""
/random/setSeeds {seed} {seed}
/RMG/Geometry/RegisterDetector Germanium detector_phys 0
/RMG/Output/ActivateOutputScheme Track
/RMG/Output/ActivateOutputScheme Staging

/run/initialize

/RMG/Output/Track/StoreAlways true
/RMG/Output/Track/StoreStageID true
/RMG/Output/Track/StoreOpticalPhotons false

/RMG/Staging/Electrons/DeferToWaitingStage true
/RMG/Staging/Electrons/VolumeSafety {safety_cm} cm
/RMG/Staging/Electrons/MaxEnergyThresholdForStacking {threshold_mev} MeV
/RMG/Staging/Electrons/AddVolumeName {stack_volume}

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position 0.0 0.0 {z_cm} cm
/gps/particle {particle}
/gps/energy {energy_mev} MeV
/gps/direction 0 0 -1

/run/beamOn {events}
"""


def _gamma_macro(
    *,
    seed: int,
    threshold_mev: float,
    safety_cm: float,
    events: int,
) -> str:
    return f"""
/random/setSeeds {seed} {seed}
/RMG/Geometry/RegisterDetector Germanium detector_phys 0
/RMG/Output/ActivateOutputScheme Track
/RMG/Output/ActivateOutputScheme Staging

/run/initialize

/RMG/Output/Track/StoreAlways true
/RMG/Output/Track/StoreStageID true
/RMG/Output/Track/StoreOpticalPhotons false

/RMG/Staging/Electrons/DeferToWaitingStage true
/RMG/Staging/Electrons/VolumeSafety {safety_cm} cm
/RMG/Staging/Electrons/MaxEnergyThresholdForStacking {threshold_mev} MeV
/RMG/Staging/Electrons/AddVolumeName world_vol

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position 0 0 20 cm
/gps/particle gamma
/gps/energy 2.6 MeV
/gps/ang/type iso

/run/beamOn {events}
"""


def _electron_suspension_macro(
    *,
    seed: int,
    suspend_on_drop: bool,
    threshold_mev: float,
    events: int,
) -> str:
    suspend = "true" if suspend_on_drop else "false"
    return f"""
/random/setSeeds {seed} {seed}
/RMG/Geometry/RegisterDetector Germanium detector_phys 0

/RMG/Output/ActivateOutputScheme Track
/RMG/Output/ActivateOutputScheme Staging

/run/initialize

/RMG/Output/Track/StoreAlways true
/RMG/Output/Track/StoreStageID true
/RMG/Output/Track/StoreOpticalPhotons false

/RMG/Staging/Electrons/DeferToWaitingStage true
/RMG/Staging/Electrons/VolumeSafety 0 cm
/RMG/Staging/Electrons/MaxEnergyThresholdForStacking {threshold_mev} MeV
/RMG/Staging/Electrons/SuspendOnEnergyDrop {suspend}
/RMG/Staging/Electrons/AddVolumeName world_vol

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position 0 0 5 cm
/gps/particle gamma
/gps/energy 2.6 MeV
/gps/direction 0 0 -1

/run/beamOn {events}
"""


def _gamma_suspension_macro(
    *,
    seed: int,
    suspend_on_drop: bool,
    threshold_mev: float,
    events: int,
) -> str:
    suspend = "true" if suspend_on_drop else "false"
    return f"""
/random/setSeeds {seed} {seed}
/RMG/Geometry/RegisterDetector Germanium detector_phys 0

/RMG/Output/ActivateOutputScheme Track
/RMG/Output/ActivateOutputScheme Staging

/run/initialize

/RMG/Output/Track/StoreAlways true
/RMG/Output/Track/StoreStageID true
/RMG/Output/Track/StoreOpticalPhotons false

/RMG/Staging/Gammas/DeferToWaitingStage true
/RMG/Staging/Gammas/VolumeSafety 0 cm
/RMG/Staging/Gammas/MaxEnergyThresholdForStacking {threshold_mev} MeV
/RMG/Staging/Gammas/SuspendOnEnergyDrop {suspend}
/RMG/Staging/Gammas/AddVolumeName world_vol

/RMG/Output/NtuplePerDetector true
/RMG/Output/NtupleUseVolumeName true

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position 0 0 5 cm
/gps/particle e-
/gps/energy 10 MeV
/gps/direction 0 0 -1

/run/beamOn {events}
"""


def test_primary_electron_respects_volume_selection():
    """Test that primary electrons are not deferred to stage 1 when the volume stacker is registered in a different volume."""
    events = scaled_events(120)
    output = "stacking-primary-volume-selection.lh5"
    macro = _primary_macro(
        seed=3000,
        particle="e-",
        energy_mev=0.1,
        safety_cm=1.0,
        threshold_mev=5.0,
        z_cm=100.0,
        events=events,
        stack_volume="detector_vol",
    )

    run_macro(macro, output)
    tracks = read_tracks(output)
    assert tracks is not None, "Missing track output"
    assert "stageid" in tracks.fields, "Missing stageid column"

    primary_stage0 = count_tracks_by_stage(
        tracks,
        pdg=11,
        stage=0,
        primary_only=True,
    )
    primary_stage1 = count_tracks_by_stage(
        tracks,
        pdg=11,
        stage=1,
        primary_only=True,
    )
    assert primary_stage0 == events
    assert primary_stage1 == 0


def test_primary_positrons_are_not_deferred_to_stage_one():
    """Test that primary positrons are not deferred to stage 1, since they can produce secondaries via annihilation at rest."""
    events = scaled_events(120)
    output = "stacking-primary-positron.lh5"
    macro = _primary_macro(
        seed=3000,
        particle="e+",
        energy_mev=1.0,
        safety_cm=1.0,
        threshold_mev=5.0,
        z_cm=20.0,
        events=events,
    )

    run_macro(macro, output)
    tracks = read_tracks(output)
    assert tracks is not None, "Missing track output"
    assert "stageid" in tracks.fields, "Missing stageid column"

    primary_stage0 = count_tracks_by_stage(
        tracks,
        pdg=-11,
        stage=0,
        primary_only=True,
    )
    primary_stage1 = count_tracks_by_stage(
        tracks,
        pdg=-11,
        stage=1,
        primary_only=True,
    )

    assert primary_stage0 == events
    assert primary_stage1 == 0


def test_secondary_electron_production_responds_to_threshold():
    """Test that secondary electron production responds to the energy threshold."""
    events = scaled_events(250)

    high_threshold_output = "stacking-secondaries-threshold-high.lh5"
    low_threshold_output = "stacking-secondaries-threshold-low.lh5"

    high_threshold_macro = _gamma_macro(
        seed=4000,
        threshold_mev=-1.0,
        safety_cm=1.0,
        events=events,
    )
    low_threshold_macro = _gamma_macro(
        seed=4001,
        threshold_mev=0.01,
        safety_cm=1.0,
        events=events,
    )

    run_macro(high_threshold_macro, high_threshold_output)
    run_macro(low_threshold_macro, low_threshold_output)

    tracks_high = read_tracks(high_threshold_output)
    tracks_low = read_tracks(low_threshold_output)

    assert tracks_high is not None, "Missing high-threshold track output"
    assert tracks_low is not None, "Missing low-threshold track output"

    secondary_stage1_high = count_tracks_by_stage(
        tracks_high,
        pdg=11,
        stage=1,
        primary_only=False,
    )
    secondary_stage1_low = count_tracks_by_stage(
        tracks_low,
        pdg=11,
        stage=1,
        primary_only=False,
    )

    assert secondary_stage1_high > 0
    assert secondary_stage1_high > secondary_stage1_low


def test_secondary_electron_production_responds_to_safety():
    """Test that secondary electron production responds to the safety distance."""
    events = scaled_events(250)

    low_safety_output = "stacking-secondaries-safety-low.lh5"
    high_safety_output = "stacking-secondaries-safety-high.lh5"

    low_safety_macro = _gamma_macro(
        seed=5000,
        threshold_mev=-1.0,
        safety_cm=0.0,
        events=events,
    )
    high_safety_macro = _gamma_macro(
        seed=5001,
        threshold_mev=-1.0,
        safety_cm=30.0,
        events=events,
    )

    run_macro(low_safety_macro, low_safety_output)
    run_macro(high_safety_macro, high_safety_output)

    tracks_low_safety = read_tracks(low_safety_output)
    tracks_high_safety = read_tracks(high_safety_output)

    assert tracks_low_safety is not None, "Missing low-safety track output"
    assert tracks_high_safety is not None, "Missing high-safety track output"

    secondary_stage1_low_safety = count_tracks_by_stage(
        tracks_low_safety,
        pdg=11,
        stage=1,
        primary_only=False,
    )
    secondary_stage1_high_safety = count_tracks_by_stage(
        tracks_high_safety,
        pdg=11,
        stage=1,
        primary_only=False,
    )

    assert secondary_stage1_low_safety > 0
    assert secondary_stage1_low_safety > secondary_stage1_high_safety


def test_secondary_electron_suspension_on_energy_drop():
    """Test electron suspension by comparing duplicate track IDs with suspension on/off."""
    events = scaled_events(250)

    output_on = "stacking-suspension-electrons-on.lh5"
    output_off = "stacking-suspension-electrons-off.lh5"

    macro_on = _electron_suspension_macro(
        seed=6100,
        suspend_on_drop=True,
        threshold_mev=0.5,
        events=events,
    )
    macro_off = _electron_suspension_macro(
        seed=6101,
        suspend_on_drop=False,
        threshold_mev=0.5,
        events=events,
    )

    run_macro(macro_on, output_on)
    run_macro(macro_off, output_off)

    tracks_on = read_tracks(output_on)
    tracks_off = read_tracks(output_off)

    assert tracks_on is not None, "Missing suspension-on track output"
    assert tracks_off is not None, "Missing suspension-off track output"
    assert "trackid" in tracks_on.fields, "Missing trackid column"
    assert "trackid" in tracks_off.fields, "Missing trackid column"

    duplicates_on = count_duplicate_trackids(
        tracks_on,
        pdg=11,
        primary_only=False,
    )
    duplicates_off = count_duplicate_trackids(
        tracks_off,
        pdg=11,
        primary_only=False,
    )

    assert duplicates_on > 0
    assert duplicates_off == 0


def test_secondary_gamma_suspension_on_energy_drop():
    """Test gamma suspension by comparing duplicate track IDs with suspension on/off."""
    events = scaled_events(250)

    output_on = "stacking-suspension-gammas-on.lh5"
    output_off = "stacking-suspension-gammas-off.lh5"

    macro_on = _gamma_suspension_macro(
        seed=6200,
        suspend_on_drop=True,
        threshold_mev=1.0,
        events=events,
    )
    macro_off = _gamma_suspension_macro(
        seed=6201,
        suspend_on_drop=False,
        threshold_mev=1.0,
        events=events,
    )

    run_macro(macro_on, output_on)
    run_macro(macro_off, output_off)

    tracks_on = read_tracks(output_on)
    tracks_off = read_tracks(output_off)

    assert tracks_on is not None, "Missing suspension-on track output"
    assert tracks_off is not None, "Missing suspension-off track output"
    assert "trackid" in tracks_on.fields, "Missing trackid column"
    assert "trackid" in tracks_off.fields, "Missing trackid column"

    duplicates_on = count_duplicate_trackids(
        tracks_on,
        pdg=22,
        primary_only=False,
    )
    duplicates_off = count_duplicate_trackids(
        tracks_off,
        pdg=22,
        primary_only=False,
    )

    assert duplicates_on > 0
    assert duplicates_off == 0
