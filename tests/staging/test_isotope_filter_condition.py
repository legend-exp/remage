from __future__ import annotations

from staging_test_utils import count_tracks_by_stage, read_tracks, run_macro


def _isotope_filter_macro(
    *, seed: int, isotope_a: int, isotope_z: int, events: int = 1
) -> str:
    return f"""
/random/setSeeds {seed} {seed}
/RMG/Output/ActivateOutputScheme Track
/RMG/Output/ActivateOutputScheme Staging
/RMG/Output/ActivateOutputScheme IsotopeFilter
/RMG/Geometry/RegisterDetector Germanium detector_phys 0


/RMG/Processes/HadronicPhysics Shielding

/run/initialize

/RMG/Output/Track/StoreStageID true

/RMG/Output/IsotopeFilter/AddIsotope {isotope_a} {isotope_z}
/RMG/Staging/OpticalPhotons/DeferToWaitingStage true
/RMG/Output/IsotopeFilter/DiscardWaitingTracksUnlessIsotopeProduced true

/RMG/Staging/Electrons/DeferToWaitingStage true
/RMG/Staging/Electrons/VolumeSafety 0.0 cm
/RMG/Staging/Electrons/MaxEnergyThresholdForStacking 10.0 MeV
/RMG/Staging/Electrons/AddVolumeName world

/RMG/Generator/Confine UnConfined
/RMG/Generator/Select GPS
/gps/position 50.0 50.0 50.0 cm
/gps/particle neutron
/gps/energy 0.025 eV
/gps/direction 0 0 1

/run/beamOn {events}
"""


def test_ar41_capture_keeps_event():
    output = "isotope-filter-ar41.lh5"
    seed = 9101

    macro = _isotope_filter_macro(seed=seed, isotope_a=41, isotope_z=18, events=1)
    run_macro(macro, output, gdml_file="gdml/geometry-isotope-ar40.gdml")

    tracks = read_tracks(output)
    assert tracks is not None, "Missing track output"
    assert "stageid" in tracks.fields, "Missing stageid column"
    assert len(tracks) > 0, "Ar41 event should be retained"
    assert any(int(pdg) == 1000180410 for pdg in tracks.particle), (
        "Expected Ar41 track in retained event"
    )

    # Secondary electrons in world should be deferred by the central staging scheme to stage 1.
    deferred_secondary_electrons = count_tracks_by_stage(
        tracks, pdg=11, stage=1, primary_only=False
    )
    assert deferred_secondary_electrons > 0


def test_ar39_filter_discards_event():
    output = "isotope-filter-ar39.lh5"
    seed = 9102

    macro = _isotope_filter_macro(seed=seed, isotope_a=39, isotope_z=18, events=1)
    run_macro(macro, output, gdml_file="gdml/geometry-isotope-ar40.gdml")

    tracks = read_tracks(output)
    # Secondary electrons in world should be deferred by the central staging scheme to stage 1.
    deferred_secondary_electrons = count_tracks_by_stage(
        tracks, pdg=11, stage=1, primary_only=False
    )
    assert tracks is not None, "Missing track output"
    assert "stageid" in tracks.fields, "Missing stageid column"
    assert deferred_secondary_electrons == 0
