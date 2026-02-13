from __future__ import annotations

from pathlib import Path

import awkward as ak
import hist
import matplotlib.pyplot as plt
import numpy as np
from lgdo import lh5

# simulated energies in keV
energies = np.concatenate(
    [
        np.arange(30, 100, step=20),
        np.arange(100, 500, step=200),
        np.arange(500, 5000, step=500),
    ]
)

mats = {
    "ge": 5.323,  # g/cm3, room temperature
    "cu": 8.96,  # g/cm3
    "ar": 1.39,  # g/cm3
}


def read_estar(mat: str):
    columns = [
        "kinetic_energy",
        "collision_stp",
        "radiative_stp",
        "total_stp",
        "csda_range",
        "radiation_yield",
    ]

    with Path(f"data/estar-{mat}.txt").open() as f:
        lines = f.readlines()

    # Find the start of numeric data (skip headers)
    data_lines = []
    for line in lines:
        if line.strip() and line[0].isdigit():  # Data lines start with a number
            data_lines.append(line.strip())

    # Convert the extracted data to a NumPy array
    values = np.array([list(map(float, line.split())) for line in data_lines])

    # Convert to an Awkward Array
    a = ak.Array({col: values[:, i] for i, col in enumerate(columns)})[:43]
    a["x"] = 1000 * a.kinetic_energy  # in mm
    return a


def ak_diff(data):
    return data[..., :-1] - data[..., 1:]


def calc_range(filename):
    data = lh5.read_as("/stp/det001", filename, "ak")

    # select only track 1, we are not interested in secondaries
    data = data[data.trackid == 1]

    # make events
    data = ak.unflatten(data, ak.run_lengths(data.evtid))

    # convert to mm
    return 1000 * ak.sum(
        np.sqrt(
            ak_diff(data.xloc) ** 2 + ak_diff(data.yloc) ** 2 + ak_diff(data.zloc) ** 2
        ),
        axis=-1,
    )


def calc_stopping_power(filename):
    data = lh5.read_as("/stp/det001", filename, "ak")

    # select only track 1, we are not interested in secondaries
    data = data[data.trackid == 1]

    # make events
    data = ak.unflatten(data, ak.run_lengths(data.evtid))[..., :2]

    stp_len = np.sqrt(
        ak_diff(data.xloc) ** 2 + ak_diff(data.yloc) ** 2 + ak_diff(data.zloc) ** 2
    )

    # convert to keV / mm
    return data.edep[..., 0] / stp_len / 1000


# tests
# =====


def test_electron_range_distributions(mat: str):
    fig, ax = plt.subplots()

    for energy in [300, 500, 1000, 1500, 2000, 3000, 4000]:  # in keV
        eranges = calc_range(f"electrons-{mat}-{energy}-keV.lh5")
        # convert to mm
        h = hist.new.Reg(200, 0, 7, name="Electron integrated path [mm]").Double()
        h.fill(eranges)
        h.plot(ax=ax, yerr=False, label="$E^{kin}_e$" + f" = {energy} keV")

    ax.set_ylabel("Density")
    ax.legend()
    ax.grid()
    fig.savefig(f"e-range-{mat}-distributions.output.png")


def test_electron_range_vs_energy(mat: str, density: float, estar_data: ak.Array):
    fig, ax = plt.subplots()

    ax.plot(
        estar_data.x,
        10 * estar_data.csda_range / density,  # convert to mm
        label="CSDA range (ESTAR)",
    )

    ax.plot(
        energies,
        [ak.mean(calc_range(f"electrons-{mat}-{e}-keV.lh5")) for e in energies],
        c="black",
        marker="o",
        lw=0,
        ms=3,
        label=f"$remage$ simulation: $e^-$ in {mat.title()}",
    )

    ax.set_xlim(0, 5000)
    ax.set_xlabel("Electron kinetic energy [keV]")
    ax.set_ylabel("Electron integrated path [mm]")
    ax.legend()
    ax.grid()
    fig.savefig(f"e-range-{mat}-vs-estar.output.png")


def test_electron_stopping_power_vs_energy(
    mat: str, density: float, estar_data: ak.Array
):
    fig, ax = plt.subplots()

    ax.plot(
        estar_data.x,
        100 * estar_data.radiative_stp * density,  # convert to keV/mm
        ls="--",
        label="Radiative stopping power (ESTAR)",
    )

    ax.plot(
        estar_data.x,
        100 * estar_data.collision_stp * density,
        ls="--",
        label="Collisional stopping power (ESTAR)",
    )

    ax.plot(
        estar_data.x,
        100 * estar_data.total_stp * density,
        label="Total stopping power (ESTAR)",
    )

    ax.plot(
        energies,
        [
            ak.mean(calc_stopping_power(f"electrons-{mat}-{e}-keV.lh5"))
            for e in energies
        ],
        c="black",
        marker="o",
        lw=0,
        ms=3,
        label=f"$remage$ simulation: $e^-$ in {mat.title()}",
    )

    ax.set_xlabel("Electron kinetic energy [keV]")
    ax.set_ylabel("Stopping power [keV / mm]")
    ax.legend()
    ax.grid()
    ax.set_xscale("log")

    fig.savefig(f"e-stopping-power-{mat}-vs-estar.output.png")


for mat, density in mats.items():
    estar_data = read_estar(mat)
    test_electron_range_distributions(mat)
    test_electron_range_vs_energy(mat, density, estar_data)
    test_electron_stopping_power_vs_energy(mat, density, estar_data)
