from __future__ import annotations

import numpy as np
from lgdo import lh5
from remage import remage_run


def run_sim(run_name: str, cmd: str, gdml: str):
    macro = f"""
    /RMG/Geometry/RegisterDetector Scintillator Box1 1
    /RMG/Geometry/RegisterDetector Scintillator Box2 2

    /RMG/Output/NtupleUseVolumeName true

    /run/initialize

    /RMG/Generator/Confine Volume
    /RMG/Generator/Confinement/ForceContainmentCheck true
    /RMG/Generator/Confinement/Physical/AddVolume Box1
    /RMG/Generator/Confinement/Physical/AddVolume Box2

    {cmd}

    /RMG/Generator/Select GPS
    /gps/particle e-
    /gps/energy 1 eV

    /run/beamOn 20000"""

    remage_run(
        macro.split("\n"),
        gdml_files=gdml,
        output=f"{run_name}.lh5",
        overwrite_output=True,
    )


# Define the different detector registration commands to test
gdml_default = "gdml/geometry_weight.gdml"
runs = [
    (
        "test-weight-volume",
        "/RMG/Generator/Confinement/SampleWeightByMass false",
    ),
    (
        "test-weight-mass",
        "/RMG/Generator/Confinement/SampleWeightByMass true",
    ),
    (
        "test-weight-isotope",
        "/RMG/Generator/Confinement/SampleWeightByMassIsotope 32 76",
    ),
]
file_vols = "test-weight-volume.lh5"
file_mass = "test-weight-mass.lh5"
file_isot = "test-weight-isotope.lh5"

# Run each simulation
for run_name, register_command in runs:
    run_sim(run_name, register_command, gdml_default)


def get_ratio(file):
    box1 = lh5.read_as("/stp/Box1", file, "ak")
    box2 = lh5.read_as("/stp/Box2", file, "ak")
    print(file, len(box1) / len(box2))
    return len(box1) / len(box2)


exp_vol = 1
assert np.abs(get_ratio(file_vols) - exp_vol) / exp_vol < 0.1
exp_mass = 10 / 5
assert np.abs(get_ratio(file_mass) - exp_mass) / exp_mass < 0.1
exp_isot = (10 * 0.5) / (5 * 0.25)
assert np.abs(get_ratio(file_isot) - exp_isot) / exp_isot < 0.1
