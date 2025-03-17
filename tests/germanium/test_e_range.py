from __future__ import annotations

from pathlib import Path

import awkward as ak
import hist
import matplotlib.pyplot as plt
import numpy as np
from lgdo import lh5


def read_estar():
    columns = [
        "kinetic_energy",
        "collision_stp",
        "radiative_stp",
        "total_stp",
        "csda_range",
        "radiation_yield",
    ]

    with Path("data/estar-ge.txt").open() as f:
        lines = f.readlines()

    # Find the start of numeric data (skip headers)
    data_lines = []
    for line in lines:
        if line.strip() and line[0].isdigit():  # Data lines start with a number
            data_lines.append(line.strip())

    # Convert the extracted data to a NumPy array
    values = np.array([list(map(float, line.split())) for line in data_lines])

    # Convert to an Awkward Array
    return ak.Array({col: values[:, i] for i, col in enumerate(columns)})


def ak_diff(data):
    return data[..., :-1] - data[..., 1:]


def calc_range(filename):
    data = lh5.read_as("/stp/det001", filename, "ak")

    # make events
    data = ak.unflatten(data, ak.run_lengths(data.evtid))

    e_steps = data[data.particle == 11]
    return ak.sum(
        np.sqrt(
            ak_diff(e_steps.xloc) ** 2
            + ak_diff(e_steps.yloc) ** 2
            + ak_diff(e_steps.yloc) ** 2
        ),
        axis=-1,
    )


fig, ax = plt.subplots(figsize=(7, 4))

for energy in [500, 1000, 1500, 2000, 3000, 4000]:  # in keV
    eranges = calc_range(f"electrons-ge-{energy}-keV.lh5")
    # convert to mm
    h = hist.new.Reg(200, 0, 8, name="Electron range [mm]").Double()
    h.fill(eranges * 1000)
    h.plot(ax=ax, yerr=False, label=f"{energy} keV")

ax.legend()
fig.savefig("e-range-ge-distributions.output.pdf")

fig, ax = plt.subplots(figsize=(7, 4))

energies = np.concatenate(
    [np.arange(100, 2000, step=100), np.arange(2000, 5000, step=500)]
)

y_ranges = []
y_ranges_unc = []
for energy in energies:  # in keV
    r = 1000 * calc_range(f"electrons-ge-{energy}-keV.lh5")
    y_ranges.append(ak.mean(r))
    y_ranges_unc.append(ak.std(r) / np.sqrt(len(r)))

ax.errorbar(
    energies,
    y_ranges,
    yerr=y_ranges_unc,
    fmt="o",
    markersize=3,
    capsize=3,
    label="Simulation: $e^-$ in Ge",
)

estar_data = read_estar()

nat_ge_density = 5.323  # g/cm3, room temperature
ax.plot(
    1000 * estar_data.kinetic_energy,
    10 * estar_data.csda_range / nat_ge_density,
    label="CSDA range (ESTAR)",
)

ax.set_xlim(0, 5000)
ax.set_xlabel("Electron kinetic energy [keV]")
ax.set_ylabel("Linear range [mm]")
ax.legend()
fig.savefig("e-range-ge-vs-estar.output.pdf")
