from __future__ import annotations

import awkward as ak
import hist
import matplotlib.pyplot as plt
import numpy as np
from lgdo import lh5
from remage import remage_run

macro = """
/RMG/Output/ActivateOutputScheme Track
/RMG/Output/ActivateOutputScheme ParticleFilter

/RMG/Processes/EnableGammaAngularCorrelation {angcorr}

/run/initialize

/RMG/Output/ParticleFilter/AddParticle 11
/RMG/Output/ParticleFilter/AddKillVolume vacuum

/RMG/Generator/Confine UnConfined

/RMG/Generator/Select GPS
/gps/position     0 0 0
/gps/particle     ion
/gps/ion          {Z} {A} 0 {level}
/gps/energy       0 keV
/gps/ang/type     iso

/run/beamOn {events}
"""


def simulate(Z, A, level, angcorr):
    output = f"output-{Z}-{A}-{level}-angcorr-{angcorr}.lh5"

    events = 100000

    remage_run(
        macro.split("\n"),
        macro_substitutions={
            "Z": Z,
            "A": A,
            "level": level,
            "angcorr": angcorr,
            "events": events,
        },
        gdml_files="gdml/geometry.gdml",
        output=output,
        overwrite_output=True,
        log_level="summary",
    )

    return output


def _expectation_co60(cos_theta):
    return 1 + 1 / 8 * cos_theta**2 + 1 / 24 * cos_theta**4


def test_plot_gammacorr():
    level = 0

    items = [
        (27, 60, "$^{60}$Co: 1.17 MeV vs. 1.33 MeV", _expectation_co60),
        (81, 208, "$^{208}$Tl: 0.58 MeV vs. 2.6 MeV", None),
    ]

    for Z, A, title, expectation_func in items:
        fig, ax = plt.subplots()

        data = {}
        for angcorr in (True, False):
            remage_output = simulate(Z, A, level, angcorr)
            # read in track data
            tracks = lh5.read_as("tracks", remage_output, library="ak")

            # read in dictionary with process ids, to filter later
            processes = lh5.read("processes", remage_output)

            # group-by event id
            trk = ak.unflatten(tracks, ak.run_lengths(tracks.evtid))

            # select gamma particles created by a radioactive decay
            decay_procid = processes[
                (
                    "RadioactiveDecay"
                    if "RadioactiveDecay" in processes
                    else "Radioactivation"
                )
            ].value
            gtrk = trk[(trk.particle == 22) & (trk.procid == decay_procid)]

            # keep only events with two gammas
            gtrk = gtrk[ak.num(gtrk) == 2]

            # gtrk has shape (n_events, 2) and fields .px, .py, .pz
            # Split into the two momenta per event:
            p1 = gtrk[:, 0]
            p2 = gtrk[:, 1]

            # Dot product
            dot = p1.px * p2.px + p1.py * p2.py + p1.pz * p2.pz

            # Norms
            norm1 = np.sqrt(p1.px**2 + p1.py**2 + p1.pz**2)
            norm2 = np.sqrt(p2.px**2 + p2.py**2 + p2.pz**2)

            # Cosine of angle
            cos_theta = dot / (norm1 * norm2)

            h = hist.new.Reg(50, -1, 1).Double().fill(cos_theta)
            data[angcorr] = h.view(flow=False)

            h.plot(
                ax=ax, yerr=False, label=rf"$\gamma$ angular correlations = {angcorr}"
            )

        # this is the expectation for 60Co and 108Tl
        cos_theta = np.linspace(-1, 1)
        if expectation_func is not None:
            expectation = expectation_func(cos_theta)
            expectation_normed = data[True].mean() / expectation.mean() * expectation
            plt.plot(
                cos_theta,
                expectation_normed,
                label=r"$\frac{1}{8}\cos(\theta)^2 + \frac{1}{24}\cos(\theta)^4$",
            )
        plt.plot(
            cos_theta,
            data[True].mean() * np.ones_like(cos_theta),
            label=r"uniform",
            linestyle="--",
        )

        ax.set_title(title)
        ax.set_xlabel(r"$\cos(\theta)$")
        ax.legend()
        fig.savefig(f"gamma-angular-distribution-{Z}-{A}.output.png")
