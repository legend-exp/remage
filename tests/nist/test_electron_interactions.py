from __future__ import annotations

from pathlib import Path

import awkward as ak
import hist
import lh5
import matplotlib.cm
import matplotlib.colors
import matplotlib.pyplot as plt
import numpy as np

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


def calc_range(filename: str):
    data = lh5.read_as("/stp/det001", filename, "ak")
    vtx = lh5.read_as("/vtx", filename, "ak")

    # select only track 1, we are not interested in secondaries
    data = data[data.trackid == 1]

    # make events
    data = ak.unflatten(data, ak.run_lengths(data.evtid))

    def stp_len_field(field: str):
        # inject the first position from the vtx table before the first edep
        vtx_loc = vtx[field][(data.evtid[..., 0] == vtx.evtid)]
        vtx_loc = ak.unflatten(vtx_loc, ak.ones_like(vtx_loc, dtype=int))
        loc = ak.concatenate([vtx_loc, data[field]], axis=1)

        return ak_diff(loc) ** 2

    # convert to mm
    return 1000 * ak.sum(
        np.sqrt(stp_len_field("xloc") + stp_len_field("yloc") + stp_len_field("zloc")),
        axis=-1,
    )


def calc_stopping_power(filename: str):
    data = lh5.read_as("/stp/det001", filename, "ak")
    vtx = lh5.read_as("/vtx", filename, "ak")

    # select only track 1, we are not interested in secondaries
    data = data[data.trackid == 1]

    # make events
    # we are only interested in the first step for each event, as we will
    # inject the first position from the vtx table.
    data = ak.unflatten(data, ak.run_lengths(data.evtid))[..., :1]

    def stp_len_field(field: str):
        # inject the first position from the vtx table before the first edep
        vtx_loc = vtx[field][(data.evtid[..., 0] == vtx.evtid)]
        vtx_loc = ak.unflatten(vtx_loc, ak.ones_like(vtx_loc, dtype=int))
        loc = ak.concatenate([vtx_loc, data[field]], axis=1)

        # we should now have 2 positions per event.
        assert ak.all(ak.num(loc, axis=-1) <= 2)
        return ak_diff(loc) ** 2

    stp_len = np.sqrt(
        stp_len_field("xloc") + stp_len_field("yloc") + stp_len_field("zloc")
    )
    assert ak.all(stp_len > 0)
    assert not ak.any(np.isnan(data.edep[..., 0]))

    # convert to keV / mm
    stop = data.edep[..., 0] / stp_len / 1000
    stop = ak.flatten(stop)
    assert not ak.any(np.isnan(stop))

    return stop


# tests
# =====


def test_electron_range_distributions(mat: str):
    fig, ax = plt.subplots()

    eranges = {}
    rmax = 3
    for step_size in range_step_sizes:  # in m
        for energy in range_energies:  # in keV
            eranges[(step_size, energy)] = calc_range(
                f"output/electrons-range-{mat}-{energy}-keV-{step_size}-m.lh5"
            )
            rmax = max(rmax, ak.max(eranges[(step_size, energy)]))

    for step_size in range_step_sizes:  # in m
        for energy in range_energies:  # in keV
            h = hist.new.Reg(
                200, 0, rmax + 0.5, name="Electron integrated path [mm]"
            ).Double()
            h.fill(eranges[(step_size, energy)])
            h.plot(ax=ax, yerr=False, label="$E^{kin}_e$" + f" = {energy:.1f} keV")

    ax.set_ylabel("Density")
    ax.legend()
    ax.grid()
    fig.savefig(f"e-range-{mat}-distributions.output.png")


def test_electron_range_vs_energy(mat: str, density: float, estar_data: ak.Array):
    fig, ax = plt.subplots()

    combined_e = []
    combined_range = []
    combined_range_err = []
    combined_step = []
    for step_size in step_sizes:  # in m
        combined_e.extend(energies)
        combined_step.extend([step_size] * len(energies))
        for e in energies:
            rn = calc_range(f"output/electrons-{mat}-{e}-keV-{step_size}-m.lh5")
            combined_range.append(ak.mean(rn))
            combined_range_err.append(ak.std(rn) / np.sqrt(ak.count(rn)))

    combined_e = np.array(combined_e)
    combined_range = np.array(combined_range)
    combined_range_err = np.array(combined_range_err)
    combined_step = np.array(combined_step)

    # inset axis.
    axin = ax.inset_axes([0.5, 0.1, 0.45, 0.45])

    for a in (axin, ax):
        a.plot(
            estar_data.x,
            10 * estar_data.csda_range / density,  # convert to mm
            label="CSDA range (ESTAR)",
        )
        nm = matplotlib.colors.LogNorm()
        step_colors = matplotlib.cm.viridis(nm(combined_step))
        sm = matplotlib.cm.ScalarMappable(norm=nm, cmap=matplotlib.cm.viridis)
        a.scatter(
            combined_e,
            combined_range,
            c=step_colors,
            marker="o",
            s=3**2,
            label=f"$remage$ simulation: $e^-$ in {mat.title()}",
        )
        for e, s, err, c in zip(
            combined_e, combined_range, combined_range_err, step_colors, strict=True
        ):
            a.errorbar(e, s, yerr=err, color=c, linewidth=1, linestyle="", zorder=0)

        a.grid()

    x_max = 4000
    axin.set_xlim(-0.1 * x_max, 1.1 * x_max)
    y_max = np.max(combined_range[combined_e < x_max])
    axin.set_ylim(-0.1 * y_max, 1.1 * y_max)

    plt.colorbar(sm, ax=ax, label="maximum step length [m]")

    ax.set_xlabel("Electron kinetic energy [keV]")
    ax.set_ylabel("Electron integrated path [mm]")
    leg = ax.legend()
    leg.legend_handles[1].set(array=None, ec="black", fc="black")
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

    estar_e_over_range = estar_data.x / (
        10 * estar_data.csda_range / density
    )  # convert to mm
    ax.plot(
        estar_data.x[estar_data.x < 1000],
        estar_e_over_range[estar_data.x < 1000],
        label="E / CSDA range (ESTAR)",
        linestyle=":",
    )

    combined_e = []
    combined_stop = []
    combined_step = []
    combined_stop_err = []
    for step_size in step_sizes:  # in m
        combined_e.extend(energies)
        combined_step.extend([step_size] * len(energies))
        for e in energies:
            sp = calc_stopping_power(
                f"output/electrons-{mat}-{e}-keV-{step_size}-m.lh5"
            )
            combined_stop.append(ak.mean(sp))
            combined_stop_err.append(ak.std(sp) / np.sqrt(ak.count(sp)))

    nm = matplotlib.colors.LogNorm()
    step_colors = matplotlib.cm.viridis(nm(combined_step))
    sm = matplotlib.cm.ScalarMappable(norm=nm, cmap=matplotlib.cm.viridis)
    ax.scatter(
        combined_e,
        combined_stop,
        c=step_colors,
        marker="o",
        s=3**2,
        label=f"$remage$ simulation: $e^-$ in {mat.title()}",
        zorder=100,
    )
    for e, s, err, c in zip(
        combined_e, combined_stop, combined_stop_err, step_colors, strict=True
    ):
        ax.errorbar(e, s, yerr=err, color=c, linewidth=1, linestyle="", zorder=0)
    plt.colorbar(sm, ax=ax, label="maximum step length [m]")

    ax.set_xlabel("Electron kinetic energy [keV]")
    ax.set_ylabel("Stopping power [keV / mm]")
    leg = ax.legend()
    leg.legend_handles[-1].set(array=None, ec="black", fc="black")
    ax.grid()
    ax.set_xscale("log")

    fig.savefig(f"e-stopping-power-{mat}-vs-estar.output.png")


for mat, density in mats.items():
    estar_data = read_estar(mat)
    test_electron_range_distributions(mat)
    test_electron_range_vs_energy(mat, density, estar_data)
    test_electron_stopping_power_vs_energy(mat, density, estar_data)
