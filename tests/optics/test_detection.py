from __future__ import annotations

import awkward as ak
import matplotlib.pyplot as plt
from lgdo import lh5
from remage import remage_run

n_events = 3000
macro = """
/RMG/Processes/OpticalPhysics

/RMG/Geometry/RegisterDetector Optical detector 001

/run/initialize

/RMG/Generator/Confine UnConfined

/RMG/Generator/Select GPS
/gps/position     0 0 18 cm
/gps/particle     opticalphoton
/gps/energy       6 eV
/gps/direction    0 0 -1

/run/beamOn {events}
"""


def simulate(r: float, e: float):
    output = f"output-detection-{e:.2f}-{r:.2f}.lh5"

    remage_run(
        macro.split("\n"),
        macro_substitutions={
            "events": n_events,
        },
        gdml_files=f"gdml/geometry-detection-{e:.2f}-{r:.2f}.gdml",
        output=output,
        overwrite_output=True,
        log_level="summary",
    )

    return output


def test_detection():
    fig, ax = plt.subplots()

    x = []
    y = []
    for e in (0.1, 0.5, 1):
        for r in (0.1, 0.5, 0.9):
            remage_output = simulate(r, e)

            stps = lh5.read_as("stp/det001", remage_output, library="ak")
            x.append((1 - r) * e)
            y.append(ak.num(stps.wavelength, axis=0) / n_events)

    ax.plot([0, 1], [0, 1], color="black", linestyle="--", linewidth=1)
    ax.scatter(x, y, marker="x", zorder=1)

    ax.set_title("sensitive surfaces")
    ax.set_ylabel("fraction of detected photons")
    ax.set_xlabel(r"$(1-R) \cdot \epsilon$")
    fig.savefig("photon-detection.output.png")
