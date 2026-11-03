from __future__ import annotations

import re
import subprocess
import sys
from math import pi
from pathlib import Path

# Box1 is a 1 m lead cube centered at x = 2 m, Box2 a 1 m lead cube with a 0.99 m daughter
sphere = """
/RMG/Generator/Confinement/Geometrical/{}Solid Sphere
/RMG/Generator/Confinement/Geometrical/CenterPositionX 2 m
/RMG/Generator/Confinement/Geometrical/Sphere/OuterRadius 0.3 m
"""
v_sphere = 4 / 3 * pi * 30**3  # cm3
rho_pb = 11.35e-3  # kg/cm3

cases = {
    "intersection": (
        "/RMG/Generator/Confinement/SamplingMode IntersectPhysicalWithGeometrical\n"
        "/RMG/Generator/Confinement/FirstSamplingVolume Physical\n"
        "/RMG/Generator/Confinement/Physical/AddVolume Box1\n" + sphere.format("Add"),
        v_sphere,
    ),
    "subtraction": (
        "/RMG/Generator/Confinement/SamplingMode SubtractGeometrical\n"
        "/RMG/Generator/Confinement/Physical/AddVolume Box1\n"
        + sphere.format("AddExcluded"),
        100**3 - v_sphere,
    ),
    "with-daughter": (
        "/RMG/Generator/Confinement/Physical/AddVolume Box2\n",
        100**3 - 99**3,
    ),
}

for name, (cmds, v_exp) in cases.items():
    macro = f"sampled-volume-{name}.mac"
    Path(macro).write_text(f"""
/run/initialize
/RMG/Generator/Confine Volume
/RMG/Generator/Confinement/MaxSamplingTrials 1000
{cmds}
/RMG/Generator/Select GPS
/gps/particle e-
/gps/energy 1 eV
/run/beamOn 100000
""")

    out = subprocess.run(
        [
            sys.executable,
            "-m",
            "remage",
            "-t",
            "2",
            "-g",
            "gdml/geometry_boxes.gdml",
            "--",
            macro,
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        check=True,
    ).stdout

    v, v_err, m, m_err = map(
        float,
        re.search(
            r"has volume (\S+) ± (\S+) cm3 and mass (\S+) ± (\S+) kg", out
        ).groups(),
    )
    print(
        f"{name}: volume {v} ± {v_err} cm3 (expected {v_exp:.6g}), mass {m} ± {m_err} kg"
    )

    assert abs(v - v_exp) < 5 * v_err
    assert abs(m - v_exp * rho_pb) < 5 * m_err
