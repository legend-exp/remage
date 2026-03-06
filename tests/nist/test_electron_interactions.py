from __future__ import annotations

from pathlib import Path

import awkward as ak
import hist
import matplotlib.colors
import matplotlib.pyplot as plt
import numpy as np
from lgdo import lh5

# simulated energies in keV
energies = np.geomspace(30, 50000, num=15).astype(int)  # in keV
step_sizes = (1, 0.1, 0.01, 0.001, 0.0001)  # in m
range_energies = np.array([300, 500, 1000, 1500, 2000, 3000, 4000])  # in keV
range_step_sizes = (1,)  # in m

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
    a = ak.Array({col: values[:, i] for i, col in enumerate(columns)})[:60]
    a["x"] = 1000 * a.kinetic_energy  # in keV
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
    assert ak.all(stp_len > 0)
    assert not ak.any(np.isnan(data.edep[..., 0]))

    # convert to keV / mm
    stop = data.edep[..., 0] / stp_len / 1000
    assert not ak.any(np.isnan(stop))

    return stop


# tests
# =====


def test_electron_range_distributions(mat: str):
    fig, ax = plt.subplots()

    for step_size in range_step_sizes:  # in m
        for energy in range_energies:  # in keV
            eranges = calc_range(
                f"output/electrons-range-{mat}-{energy}-keV-{step_size}-m.lh5"
            )
            # convert to mm
            h = hist.new.Reg(200, 0, 7, name="Electron integrated path [mm]").Double()
            h.fill(eranges)
            h.plot(ax=ax, yerr=False, label="$E^{kin}_e$" + f" = {energy:.1f} keV")

    ax.set_ylabel("Density")
    ax.legend()
    ax.grid()
    fig.savefig(f"e-range-{mat}-distributions.output.png")


def test_electron_range_vs_energy(mat: str, density: float, estar_data: ak.Array):
    fig, ax = plt.subplots()

    combined_e = []
    combined_range = []
    combined_step = []
    for step_size in step_sizes:  # in m
        combined_e.extend(energies)
        combined_step.extend([step_size] * len(energies))
        combined_range.extend(
            [
                ak.mean(calc_range(f"output/electrons-{mat}-{e}-keV-{step_size}-m.lh5"))
                for e in energies
            ]
        )
    combined_e = np.array(combined_e)
    combined_range = np.array(combined_range)
    combined_step = np.array(combined_step)

    # inset axis.
    axin = ax.inset_axes([0.5, 0.1, 0.45, 0.45])

    for a in (axin, ax):
        a.plot(
            estar_data.x,
            10 * estar_data.csda_range / density,  # convert to mm
            label="CSDA range (ESTAR)",
        )
        sc = a.scatter(
            combined_e,
            combined_range,
            c=combined_step,
            marker="o",
            s=3**2,
            label=f"$remage$ simulation: $e^-$ in {mat.title()}",
            norm=matplotlib.colors.LogNorm(),
        )

    x_max = 4000
    axin.set_xlim(-0.1 * x_max, 1.1 * x_max)
    y_max = np.max(combined_range[combined_e < x_max])
    axin.set_ylim(-0.1 * y_max, 1.1 * y_max)

    plt.colorbar(sc, ax=ax, label="maximum step length [m]")

    ax.set_xlabel("Electron kinetic energy [keV]")
    ax.set_ylabel("Electron integrated path [mm]")
    leg = ax.legend()
    leg.legend_handles[1].set(array=None, ec="black", fc="black")
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

    combined_e = []
    combined_stop = []
    combined_step = []
    for step_size in step_sizes:  # in m
        combined_e.extend(energies)
        combined_step.extend([step_size] * len(energies))
        combined_stop.extend(
            [
                ak.mean(
                    calc_stopping_power(
                        f"output/electrons-{mat}-{e}-keV-{step_size}-m.lh5"
                    )
                )
                for e in energies
            ]
        )

    sc = ax.scatter(
        combined_e,
        combined_stop,
        c=combined_step,
        marker="o",
        s=3**2,
        label=f"$remage$ simulation: $e^-$ in {mat.title()}",
        norm=matplotlib.colors.LogNorm(),
    )
    plt.colorbar(sc, ax=ax, label="maximum step length [m]")

    ax.set_xlabel("Electron kinetic energy [keV]")
    ax.set_ylabel("Stopping power [keV / mm]")
    leg = ax.legend()
    leg.legend_handles[3].set(array=None, ec="black", fc="black")
    ax.grid()
    ax.set_xscale("log")

    fig.savefig(f"e-stopping-power-{mat}-vs-estar.output.png")


for mat, density in mats.items():
    estar_data = read_estar(mat)
    test_electron_range_distributions(mat)
    test_electron_range_vs_energy(mat, density, estar_data)
    test_electron_stopping_power_vs_energy(mat, density, estar_data)
