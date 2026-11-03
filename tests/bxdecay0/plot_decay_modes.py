from __future__ import annotations

import itertools

import awkward as ak
import hist
import lh5
import matplotlib.pyplot as plt
import numpy as np
from numpy.linalg import norm

# Q-value of the Ge76 double beta decay and energies of the Se76 levels in keV, as used by BxDecay0
Q_BB = 2039
LEVELS = {"2p1": 559, "0p1": 1122, "2p2": 1216}


def read_events(filename):
    data = lh5.read_as("/particles", filename, "ak")
    return ak.unflatten(data, ak.run_lengths(data.evtid))


# In units of keV
def get_summed_primary_ekin(filename, pdg=11):
    evt = read_events(filename)
    evt = evt[evt.particle == pdg]
    return ak.to_numpy(ak.sum(evt.ekin * 1000, axis=-1))  # convert from MeV into KeV


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

    for decay_mode, energies in zip(modes, ekins, strict=True):  # in keV
        h = hist.new.Reg(80, 0, 2200, name="Summed electron energy [keV]").Double()
        h.fill(energies)
        h.plot(ax=ax, yerr=False, label=f"{decay_mode}")

    ax.set_xlabel("Combined primary electron energy [keV]")
    ax.set_ylabel("Density")
    ax.legend(loc="upper right")
    ax.grid()

    fig.savefig("double-beta-e-combined-primary-energy.output.png")


def plot_majoron():
    # spectral index n: the summed electron energy spectrum scales as (Q - E)^n
    spectral_index = {"0vbb_M1": 1, "0vbb_M2": 2, "0vbb_M3": 3, "2vbb": 5, "0vbb_M7": 7}

    fig, ax = plt.subplots()

    means = []
    for name, n in spectral_index.items():
        energies = get_summed_primary_ekin(f"{name}.lh5")
        means.append(
            (name, np.mean(energies), np.std(energies) / np.sqrt(len(energies)))
        )

        h = hist.new.Reg(80, 0, 2100, name="Summed electron energy [keV]").Double()
        h.fill(energies)
        h.plot(ax=ax, yerr=False, density=True, label=f"{name} ($n = {n}$)")

    ax.set_xlabel("Combined primary electron energy [keV]")
    ax.set_ylabel("Probability density [1/keV]")
    ax.legend(loc="upper right")
    ax.grid()

    fig.savefig("double-beta-majoron-combined-primary-energy.output.png")

    # a larger spectral index moves the spectrum to lower energies. The mean energies must be
    # significantly different, otherwise two processes produce the same spectrum.
    for (name1, m1, s1), (name2, m2, s2) in itertools.pairwise(means):
        if m1 - m2 < 5 * np.hypot(s1, s2):
            msg = f"mean summed electron energy of {name1} ({m1:.1f} keV) is not significantly larger than for {name2} ({m2:.1f} keV)"
            raise RuntimeError(msg)


def plot_excited_states():
    runs = {
        "2vbb": (r"$2\nu\beta\beta$ to $0^+_{gs}$", 0),
        "2vbb_2p1": (r"$2\nu\beta\beta$ to $2^+_1$", LEVELS["2p1"]),
        "2vbb_0p1": (r"$2\nu\beta\beta$ to $0^+_1$", LEVELS["0p1"]),
        "2vbb_2p2": (r"$2\nu\beta\beta$ to $2^+_2$", LEVELS["2p2"]),
        "0vbb_lambda_2p1": (r"$0\nu\beta\beta\_\lambda$ to $2^+_1$", LEVELS["2p1"]),
    }

    fig, (ax_e, ax_g) = plt.subplots(2, 1, figsize=(6.4, 8))

    for i, (name, (label, level)) in enumerate(runs.items()):
        ekin_e = get_summed_primary_ekin(f"{name}.lh5", pdg=11)
        ekin_g = get_summed_primary_ekin(f"{name}.lh5", pdg=22)

        h = hist.new.Reg(105, 0, 2100, name="Summed electron energy [keV]").Double()
        h.fill(ekin_e)
        h.plot(ax=ax_e, yerr=False, density=True, color=f"C{i}", label=label)
        ax_e.axvline(Q_BB - level, ls=":", color=f"C{i}")

        if level > 0:
            h = hist.new.Reg(140, 0, 1400, name="Summed gamma energy [keV]").Double()
            h.fill(ekin_g)
            h.plot(ax=ax_g, yerr=False, color=f"C{i}", label=label)

        # the electrons share at most the Q-value minus the energy of the daughter level, and the
        # de-excitation gammas carry the energy of the level. A small fraction of the transitions
        # emits conversion electrons instead of gammas.
        if np.quantile(ekin_e, 0.99) > Q_BB - level + 1:
            msg = (
                f"{name}: summed electron energy above the endpoint {Q_BB - level} keV"
            )
            raise RuntimeError(msg)

        frac_level = np.mean(np.abs(ekin_g - level) < 2)
        if frac_level < 0.99:
            msg = f"{name}: summed gamma energy matches the level energy {level} keV in only {100 * frac_level:.1f}% of the events"
            raise RuntimeError(msg)

    ax_e.set_xlabel("Combined primary electron energy [keV]")
    ax_e.set_ylabel("Probability density [1/keV]")
    ax_e.set_yscale("log")
    ax_e.legend(loc="upper right")
    ax_e.grid()

    ax_g.set_xlabel("Combined primary gamma energy [keV]")
    ax_g.set_ylabel("Events")
    ax_g.set_yscale("log")
    ax_g.legend(loc="upper left")
    ax_g.grid()

    fig.tight_layout()
    fig.savefig("double-beta-excited-states-primary-energy.output.png")


def get_primary_electron_angle(filename):
    evt = read_events(filename)
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

    for decay_mode, angle in zip(modes, angles, strict=True):
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
plot_majoron()
plot_excited_states()
