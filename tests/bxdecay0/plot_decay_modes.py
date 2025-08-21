from __future__ import annotations

import awkward as ak
import hist
import matplotlib.pyplot as plt
import numpy as np
from lgdo import lh5
from numpy.linalg import norm


# In units of keV
def get_summed_primary_ekin(filename):
    data = lh5.read_as("/particles", filename, "ak")

    evt = ak.unflatten(data, ak.run_lengths(data.evtid))
    # Technically we would have to filter on electrons, but the neutrinos are not even created
    return ak.sum(evt.ekin * 1000, axis=-1)  # convert from MeV into KeV


def plot_energy():
    fig, ax = plt.subplots()

    ekin_0vbb = get_summed_primary_ekin("0vbb.lh5")[
        :4000
    ]  # limit to 4000 events for good looking plots
    ekin_2vbb = get_summed_primary_ekin("2vbb.lh5")
    ekin_0vbb_M1 = get_summed_primary_ekin("0vbb_M1.lh5")
    ekin_0vbb_lambda_0 = get_summed_primary_ekin("0vbb_lambda_0.lh5")[
        :2500
    ]  # limit to 2500 events for good looking plots

    modes = [
        r"$0\nu\beta\beta$",
        r"$2\nu\beta\beta$",
        r"$0\nu\beta\beta\_M1$",
        r"$0\nu\beta\beta\_\lambda0$",
    ]
    ekins = [ekin_0vbb, ekin_2vbb, ekin_0vbb_M1, ekin_0vbb_lambda_0]

    for decay_mode, energies in zip(modes, ekins):  # in keV
        h = hist.new.Reg(80, 0, 2200, name="Summed electron energy [keV]").Double()
        h.fill(energies)
        h.plot(ax=ax, yerr=False, label=f"{decay_mode}")

    ax.set_xlabel("Combined primary electron energy [keV]")
    ax.set_ylabel("Density")
    ax.legend(loc="upper right")
    ax.grid()

    fig.savefig("double-beta-e-combined-primary-energy.output.png")


def get_primary_electron_angle(filename):
    data = lh5.read_as("/particles", filename, "ak")

    evt = ak.unflatten(data, ak.run_lengths(data.evtid))
    px = evt.px
    py = evt.py
    pz = evt.pz

    # Build 3-vectors for each particle
    p1 = np.stack(
        [ak.to_numpy(px[:, 0]), ak.to_numpy(py[:, 0]), ak.to_numpy(pz[:, 0])], axis=1
    )
    p2 = np.stack(
        [ak.to_numpy(px[:, 1]), ak.to_numpy(py[:, 1]), ak.to_numpy(pz[:, 1])], axis=1
    )
    # Dot product and norms
    dot = np.einsum("ij,ij->i", p1, p2)
    norm1 = norm(p1, axis=1)
    norm2 = norm(p2, axis=1)
    # Angle in radians
    return np.arccos(np.clip(dot / (norm1 * norm2), -1.0, 1.0))


def plot_angles():
    fig, ax = plt.subplots()

    angles_0vbb = get_primary_electron_angle("0vbb.lh5")
    angles_2vbb = get_primary_electron_angle("2vbb.lh5")
    angles_0vbb_M1 = get_primary_electron_angle("0vbb_M1.lh5")
    angles_0vbb_lambda_0 = get_primary_electron_angle("0vbb_lambda_0.lh5")

    modes = [
        r"$0\nu\beta\beta$",
        r"$2\nu\beta\beta$",
        r"$0\nu\beta\beta\_M1$",
        r"$0\nu\beta\beta\_\lambda0$",
    ]
    angles = [angles_0vbb, angles_2vbb, angles_0vbb_M1, angles_0vbb_lambda_0]

    for decay_mode, angle in zip(modes, angles):
        # convert to degree
        h = hist.new.Reg(
            80, 0, 182, name="Angle between primary electrons [°]"
        ).Double()
        h.fill(angle * 180 / np.pi)
        h.plot(ax=ax, yerr=False, label=f"{decay_mode}")

    ax.set_xlabel("Angle between primary electrons [°]")
    ax.set_ylabel("Density")
    ax.legend()
    ax.grid()

    fig.savefig("double-beta-e-primary-opening-angle.output.png")


plot_angles()
plot_energy()
