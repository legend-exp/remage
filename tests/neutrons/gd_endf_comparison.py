from __future__ import annotations

import os
import zlib
from pathlib import Path

import awkward as ak
import lh5
import matplotlib.pyplot as plt
import numpy as np
import plots_basics
from argon_endf_comparison import get_latest_g4ndl
from matplotlib.lines import Line2D

# --- Load data ---
file_gd = "data/jeff-3.3_gd.lh5"
file_scan = "neutron_energy_scan.lh5"

energy_bins = lh5.read_as("/data/energy_bins", file_gd, "ak")

isotope_names = ["155", "156", "157", "158", "160"]
sig = {
    f"Gd-{iso}": lh5.read_as(f"/data/sig_{iso}", file_gd, "ak") for iso in isotope_names
}

energy_scan_tracks = lh5.read_as("/tracks", file_scan, "ak")
energy_scan_vertex = lh5.read_as("/particles", file_scan, "ak")

# --- Preprocess ---
particles = ak.to_numpy(energy_scan_tracks.particle[energy_scan_tracks.particle != 22])
vtx_energies = ak.to_numpy(energy_scan_vertex.ekin)

isotopes = np.array([plots_basics.pdg_to_isotope(p) for p in particles])
energy_bin_idx = np.digitize(vtx_energies, bins=energy_bins) - 1

# --- Initialize dictionary ---
unique_isotopes = np.unique(isotopes)
n_bins = len(energy_bins) - 1

iso_dict = {iso: np.zeros(n_bins) for iso in unique_isotopes}

# --- Fill fractions ---
for i in range(n_bins):
    mask = energy_bin_idx == i
    if not np.any(mask):
        continue

    iso_vals, counts = np.unique(isotopes[mask], return_counts=True)
    total = counts.sum()

    for iso, cnt in zip(iso_vals, counts, strict=True):
        iso_dict[iso][i] = cnt / total if cnt > 0 else np.nan

# --- Plot ---
fig, ax = plt.subplots(figsize=(8, 5))
E_centers = 0.5 * (energy_bins[:-1] + energy_bins[1:])

colors = {
    "Gd-155": "blue",
    "Gd-156": "green",
    "Gd-157": "red",
    "Gd-158": "gray",
    "Gd-160": "purple",
}

for iso, color in colors.items():
    ax.plot(E_centers, sig[iso], label=iso, color=color)
    ax.scatter(E_centers, iso_dict.get(iso, np.zeros(n_bins)), color=color, s=10)

# Legend for data source (style)
style_legend = [
    Line2D([0], [0], color="black", lw=2, label="ENDF"),
    Line2D(
        [0],
        [0],
        marker="o",
        color="black",
        linestyle="None",
        markersize=5,
        label="Simulation",
    ),
]

# First legend (isotopes)
leg1 = ax.legend(title="Isotope", loc="upper right")

# Second legend (styles)
leg2 = ax.legend(handles=style_legend, title="Source", loc="center left")

ax.add_artist(leg1)

ax.set_xscale("log")
plots_basics.style_axes(
    ax,
    "Neutron energy [MeV]",
    "Relative capture cross section",
    f"Relative nCapture JEFF-3.3 vs Shielding (Geant4 {plots_basics.G4_VERSION_NAME}) ",
)
ax.grid(True, which="both", ls="--", lw=0.5)

plt.tight_layout()
plots_basics.save(fig, "neutron_gd_cross_section.output.png")

# --- Show G4NDL

neutron_data = os.environ.get("G4NEUTRONHPDATA")

if neutron_data is None:
    data_filepath = "/opt/geant4/share/Geant4/data"
    latest_g4ndl = get_latest_g4ndl(data_filepath)
    data_filepath = Path(data_filepath) / latest_g4ndl
else:
    data_filepath = neutron_data
    latest_g4ndl = neutron_data.split("/")[-1]


def load_g4ndl_data(filepath, z, a, element):

    energy_cap = []
    sig_cap = []
    fp = Path(filepath) / "Capture" / "CrossSection" / f"{z}_{a}_{element}.z"
    if not fp.is_file():
        msg = "Missing"
        return energy_cap, sig_cap, msg

    try:
        with fp.open("rb") as f:
            compressed = f.read()
        content = zlib.decompress(compressed).decode("utf-8")
    except Exception as e:
        msg = f"Error reading G4NDL file {fp}: {e}"
        raise RuntimeError(msg) from e

    # Parse the data
    tokens = content.split()
    if len(tokens) < 3:
        msg = f"Unexpected format in G4NDL file {fp}: not enough tokens."
        raise RuntimeError(msg)

    try:
        for i in range(3, len(tokens) - 1, 2):
            energy_cap.append(float(tokens[i]) * 1e-6)  # eV -> MeV
            sig_cap.append(float(tokens[i + 1]))
    except ValueError as e:
        msg = f"Error parsing numbers in G4NDL file {fp}: {e}"
        raise RuntimeError(msg) from e

    return energy_cap, sig_cap, ""


isotope_names = ["155", "156", "157", "158", "160"]
colors = {
    "Gd-155": "blue",
    "Gd-156": "green",
    "Gd-157": "red",
    "Gd-158": "gray",
    "Gd-160": "purple",
}

fig, ax = plt.subplots()
for iso in isotope_names:
    energy, sig, msg = load_g4ndl_data(Path(data_filepath), 64, int(iso), "Gadolinium")

    ax.plot(energy, sig, label=f"Gd-{iso} {msg}", color=colors.get(f"Gd-{iso}"))

ax.set_xscale("log")
ax.set_yscale("log")
ax.legend(ncol=2)
plots_basics.style_axes(
    ax,
    "Neutron energy [MeV]",
    "Total cross section [barn]",
    f"nCapture cross sections from {latest_g4ndl} (Geant4 {plots_basics.G4_VERSION_NAME})",
)
plots_basics.save(fig, "neutron_gd_G4NDL.output.png")
