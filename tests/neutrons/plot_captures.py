from __future__ import annotations

import subprocess

import matplotlib.pyplot as plt
import numpy as np
from lgdo import lh5


def pdg_to_isotope(pdg: int) -> str:
    pdg = int(pdg)
    if abs(pdg) < 1_000_000_000:
        return f"PDG {pdg}"

    A = (pdg // 10) % 1000 - 1
    Z = (pdg // 10000) % 1000

    element = [
        "n",
        "H",
        "He",
        "Li",
        "Be",
        "B",
        "C",
        "N",
        "O",
        "F",
        "Ne",
        "Na",
        "Mg",
        "Al",
        "Si",
        "P",
        "S",
        "Cl",
        "Ar",
        "K",
        "Ca",
        "Sc",
        "Ti",
        "V",
        "Cr",
        "Mn",
        "Fe",
        "Co",
        "Ni",
        "Cu",
        "Zn",
        "Ga",
        "Ge",
        "As",
        "Se",
        "Br",
        "Kr",
        "Rb",
        "Sr",
        "Y",
        "Zr",
        "Nb",
        "Mo",
        "Tc",
        "Ru",
        "Rh",
        "Pd",
        "Ag",
        "Cd",
        "In",
        "Sn",
        "Sb",
        "Te",
        "I",
        "Xe",
        "Cs",
        "Ba",
        "La",
        "Ce",
        "Pr",
        "Nd",
        "Pm",
        "Sm",
        "Eu",
        "Gd",
    ]

    symbol = element[Z] if len(element) > Z else f"Z={Z}"
    return f"{symbol}-{A}"


def gamma_spectrum_and_sum(data, isotope):
    evt_ids = data.evtid[data.particle == isotope]

    gamma_mask = np.isin(data.evtid, evt_ids)
    gamma_evtids = data.evtid[gamma_mask]
    secondary_energies = data.ekin[gamma_mask]
    gamma_energies = data.ekin[gamma_mask & (data.particle == 22)]

    summed_energy = np.bincount(
        gamma_evtids,
        weights=secondary_energies,
    )
    summed_energy = summed_energy[summed_energy > 0]

    return gamma_energies, summed_energy


filename_evap = "neutron_captures_evap.lh5"
filename_norm = "neutron_captures_normal.lh5"

# Expected summed gamma energies [MeV] keyed by PDG code
expected_sum_energy = {
    1000010020: 2.2224,  # Capture on H-1 produces H-2
    1000180370: 8.7875,  # Capture on Ar-36 produces Ar-37
    1000180390: 6.599,  # Capture on Ar-38 produces Ar-39
    1000180410: 6.0989,  # Capture on Ar-40 produces Ar-41
    1000240510: 9.2607,  # Capture on Cr-50 produces Cr-51
    1000240530: 7.9394,  # Capture on Cr-52 produces Cr-53
    1000240540: 9.7191,  # Capture on Cr-53 produces Cr-54
    1000260550: 9.2981,  # Capture on Fe-54 produces Fe-55
    1000260570: 7.6462,  # Capture on Fe-56 produces Fe-57
    1000260580: 10.0446,  # Capture on Fe-57 produces Fe-58
    1000280590: 8.9993,  # Capture on Ni-58 produces Ni-59
    1000280610: 7.8201,  # Capture on Ni-60 produces Ni-61
    1000280630: 6.8377,  # Capture on Ni-62 produces Ni-63
    1000641560: 8.5365,  # Capture on Gd-155 produces Gd-156
    1000641580: 7.9374,  # Capture on Gd-157 produces Gd-158
}

bins = np.linspace(0, 10.6, 250)  # MeV

data_evap = lh5.read_as("/tracks", filename_evap, "ak")
data_norm = lh5.read_as("/tracks", filename_norm, "ak")

version = subprocess.run(
    ["geant4-config", "--version"],
    check=False,
    capture_output=True,
    text=True,
)
version_name = version.stdout.strip()  # remove any trailing newline

for isotope in expected_sum_energy:
    g_norm, s_norm = gamma_spectrum_and_sum(data_norm, isotope)
    g_evap, s_evap = gamma_spectrum_and_sum(data_evap, isotope)

    fig, (ax_norm, ax_evap) = plt.subplots(1, 2, figsize=(12, 5), sharey=True)

    ax_norm.hist(
        g_norm,
        bins=bins,
        histtype="step",
        linewidth=2,
        label="y spectrum",
    )
    ax_norm.hist(
        s_norm,
        bins=bins,
        histtype="step",
        linewidth=2,
        label="ΣEy per capture",
    )
    ax_norm.set_title(f"Standard Shielding {len(s_norm)} captures")
    ax_norm.set_xlabel("Energy [MeV]")
    ax_norm.set_ylabel("Counts")

    ax_evap.hist(
        g_evap,
        bins=bins,
        histtype="step",
        linewidth=2,
        label="y spectrum",
    )
    ax_evap.hist(
        s_evap,
        bins=bins,
        histtype="step",
        linewidth=2,
        label="ΣEy per capture",
    )
    ax_evap.set_title(f"G4PhotonEvaporation {len(s_evap)} captures")
    ax_evap.set_xlabel("Energy [MeV]")

    expected = expected_sum_energy.get(int(isotope))
    ax_norm.axvline(
        expected,
        color="k",
        linestyle=":",
        linewidth=2,
        label="Expected ΣEy",
    )
    ax_evap.axvline(
        expected, color="k", linestyle=":", linewidth=2, label="Expected ΣEy"
    )

    ax_norm.legend()
    ax_evap.legend()

    iso_label = pdg_to_isotope(isotope)
    fig.suptitle(f"Neutron capture on {iso_label} Geant4 {version_name}")

    ax_norm.set_yscale("log")
    plt.tight_layout()
    plt.savefig(f"neutron_capture_{iso_label.replace('-', '_')}.output.png")
