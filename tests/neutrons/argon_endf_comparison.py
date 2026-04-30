from __future__ import annotations

import os
import re
import zlib
from pathlib import Path

import lh5
import matplotlib.pyplot as plt
import numpy as np
import plots_basics


def get_latest_g4ndl(path):
    pattern = re.compile(r"G4NDL(\d+(?:\.\d+)*)")
    versions = []

    for entry in Path(path).iterdir():
        name = entry.name
        match = pattern.fullmatch(name)
        if match:
            version = tuple(map(int, match.group(1).split(".")))
            versions.append((version, name))

    if not versions:
        return None

    latest = max(versions, key=lambda x: x[0])
    return latest[1]


neutron_data = os.environ.get("G4NEUTRONHPDATA")

if neutron_data is None:
    data_filepath = "/opt/geant4/share/Geant4/data"
    latest_g4ndl = get_latest_g4ndl(data_filepath)
    data_filepath = Path(data_filepath) / latest_g4ndl
else:
    data_filepath = neutron_data
    latest_g4ndl = neutron_data.split("/")[
        -1
    ]  # just get the last part of the path as the version name


def load_g4ndl_data(filepath, energy_bins, z, a, element):

    energy_cap = []
    sig_cap = []
    energy_el = []
    sig_el = []
    energy_inel = []
    sig_inel = []

    file_targets = [
        ("Capture", energy_cap, sig_cap),
        ("Elastic", energy_el, sig_el),
        ("Inelastic", energy_inel, sig_inel),
    ]

    missing_files = []

    for channel, energy_target, sig_target in file_targets:
        fp = Path(filepath) / channel / "CrossSection" / f"{z}_{a}_{element}.z"
        if not fp.is_file():
            missing_files.append(channel)
            continue  # skip missing files

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
                energy_target.append(float(tokens[i]) * 1e-6)  # eV -> MeV
                sig_target.append(float(tokens[i + 1]))
        except ValueError as e:
            msg = f"Error parsing numbers in G4NDL file {fp}: {e}"
            raise RuntimeError(msg) from e

    # If all files are missing, just return NaNs
    if missing_files:
        msg = "missing: " + ", ".join(f[0] for f in missing_files)
        return energy_bins, np.full(len(energy_bins), 0), msg

    # Combine energies and interpolate
    combination_energy = np.unique(
        np.concatenate((energy_cap, energy_inel, energy_el, energy_bins))
    )

    sig_cap = (
        np.interp(combination_energy, energy_cap, sig_cap)
        if energy_cap
        else np.zeros_like(combination_energy)
    )
    sig_inel = (
        np.interp(combination_energy, energy_inel, sig_inel)
        if energy_inel
        else np.zeros_like(combination_energy)
    )
    sig_el = (
        np.interp(combination_energy, energy_el, sig_el)
        if energy_el
        else np.zeros_like(combination_energy)
    )

    sig_sum = sig_cap + sig_inel + sig_el

    return combination_energy, sig_sum, ""


def bin_data(energy, sig, energy_bins):
    bin_indices = np.digitize(energy, energy_bins) - 1
    sig_result = []
    for i in range(len(energy_bins) - 1):
        mask_bin = bin_indices == i
        if np.any(mask_bin):
            sig_result.append(np.mean(sig[mask_bin]))
        else:
            sig_result.append(0)  # or 0, depending on how you want to handle empty bins

    return np.array(sig_result)


def process_sim(energy_bins, a):
    data = lh5.read_as("/stp/det001", f"neutron_argon{a}_transport.lh5", "ak")
    mask = data.particle == 2112
    distance_to_surface = data.dist_to_surf[mask]  # in m

    x = data.xloc[mask]  # in m
    y = data.yloc[mask]
    z = data.zloc[mask]
    evt = data.evtid[mask]
    track = data.trackid[mask]
    velocity = data.v_pre[mask]  # in m/ns

    # Compute differences
    dx = x[1:] - x[:-1]
    dy = y[1:] - y[:-1]
    dz = z[1:] - z[:-1]
    velocity = velocity[:-1]  # in m/ns # remove the last entry to get same size

    # Valid steps: same event AND same track
    valid = (
        (evt[1:] == evt[:-1])
        & (track[1:] == track[:-1])
        & (distance_to_surface[1:] > 10)
    )  # only consider steps that end 10m away from the surface to avoid weird edge effects

    dx = dx[valid]
    dy = dy[valid]
    dz = dz[valid]
    dr = np.sqrt(dx**2 + dy**2 + dz**2)

    velocity = velocity[valid] * 1e9  # m/ns -> m/s

    m_n = 1.675e-27  # kg
    joule_to_mev = 6.242e12

    E = 0.5 * m_n * velocity**2 * joule_to_mev  # MeV

    lambda_E = np.array(bin_data(E, dr, energy_bins))

    # Convert to cross section

    NA = 6.022e23
    rho = 1.396  # g/cm^3
    M = 39.948  # g/mol

    n = rho * NA / M  # atoms/cm^3

    lambda_cm = lambda_E * 100  # m → cm

    sigma = 1 / (n * lambda_cm)  # cm^2

    return sigma / 1e-24


energy_bins = lh5.read_as("/data/energy_bins", "data/jeff-3.3_argon.lh5", "ak")
jeff_3_cross_sections_40 = lh5.read_as(
    "/data/cross_sections_40", "data/jeff-3.3_argon.lh5", "ak"
)
jeff_3_cross_sections_36 = lh5.read_as(
    "/data/cross_sections_36", "data/jeff-3.3_argon.lh5", "ak"
)
jeff_3_cross_sections_38 = lh5.read_as(
    "/data/cross_sections_38", "data/jeff-3.3_argon.lh5", "ak"
)

energy_40, sig_40, msg_40 = load_g4ndl_data(
    Path(data_filepath), energy_bins, 18, 40, "Argon"
)
energy_36, sig_36, msg_36 = load_g4ndl_data(
    Path(data_filepath), energy_bins, 18, 36, "Argon"
)
energy_38, sig_38, msg_38 = load_g4ndl_data(
    Path(data_filepath), energy_bins, 18, 38, "Argon"
)

# Also re-bin the G4NDL data to match the JEFF 3.3 binning for a direct comparison
sig_40_sum_binned = bin_data(energy_40, sig_40, energy_bins)
sig_36_sum_binned = bin_data(energy_36, sig_36, energy_bins)
sig_38_sum_binned = bin_data(energy_38, sig_38, energy_bins)

# Weight with abundance
sig_40_sum_binned_weighted = sig_40_sum_binned * 0.996
sig_36_sum_binned_weighted = sig_36_sum_binned * 0.0034
sig_38_sum_binned_weighted = sig_38_sum_binned * 0.0006


sig_sum_binned = (
    sig_40_sum_binned_weighted + sig_36_sum_binned_weighted + sig_38_sum_binned_weighted
)
energy_bins_centers = 0.5 * (energy_bins[:-1] + energy_bins[1:])

jeff_3_cross_sections_40_weighted = jeff_3_cross_sections_40 * 0.996
jeff_3_cross_sections_36_weighted = jeff_3_cross_sections_36 * 0.0034
jeff_3_cross_sections_38_weighted = jeff_3_cross_sections_38 * 0.0006

jeff_sum = (
    jeff_3_cross_sections_40_weighted
    + jeff_3_cross_sections_36_weighted
    + jeff_3_cross_sections_38_weighted
)


# Now get the sim data stuff

ar_36_sim = process_sim(energy_bins, 36)
ar_38_sim = process_sim(energy_bins, 38)
ar_40_sim = process_sim(energy_bins, 40)

ar_sum_sim = ar_40_sim * 0.996 + ar_36_sim * 0.0034 + ar_38_sim * 0.0006


fig, ax = plt.subplots()
ax.plot(
    energy_bins_centers,
    jeff_3_cross_sections_36_weighted,
    label="36-Ar JEFF 3.3",
    color="blue",
    zorder=1,
)
ax.plot(
    energy_bins_centers,
    jeff_3_cross_sections_38_weighted,
    label="38-Ar JEFF 3.3",
    color="green",
    zorder=2,
)
ax.plot(
    energy_bins_centers,
    jeff_3_cross_sections_40_weighted,
    label="40-Ar JEFF 3.3",
    color="red",
    zorder=3,
)
ax.plot(energy_bins_centers, jeff_sum, label="Sum JEFF 3.3", color="black", zorder=4)

ax.plot(
    energy_bins_centers,
    sig_36_sum_binned_weighted,
    label=f"36-Ar {msg_36}",
    color="darkblue",
    linestyle=(0, (1, 4)),
    linewidth=4,
    zorder=5,
)
ax.plot(
    energy_bins_centers,
    sig_38_sum_binned_weighted,
    label=f"38-Ar {msg_38}",
    color="darkgreen",
    linestyle=(0, (1, 4)),
    linewidth=4,
    zorder=6,
)
ax.plot(
    energy_bins_centers,
    sig_40_sum_binned_weighted,
    label=f"40-Ar {msg_40}",
    color="darkred",
    linestyle=(0, (1, 4)),
    linewidth=4,
    zorder=7,
)
ax.plot(
    energy_bins_centers,
    sig_sum_binned,
    label=f"Sum {latest_g4ndl}",
    color="darkgray",
    linestyle=(0, (1, 7)),
    linewidth=2.5,
    zorder=8,
)

ax.set_xscale("log")
ax.set_yscale("log")
ax.legend(ncol=2)
plots_basics.style_axes(
    ax,
    "Neutron energy [MeV]",
    "Total cross section [barn]",
    f"(n,tot) cross-section in Ar - JEFF 3.3 vs {latest_g4ndl}",
)
plots_basics.save(fig, "neutron_jeff_vs_g4ndl.output.png")

fig, ax = plt.subplots()
ax.plot(energy_bins_centers, jeff_sum, label="JEFF 3.3", color="black", zorder=1)

ax.plot(
    energy_bins_centers,
    sig_sum_binned,
    label=f"{latest_g4ndl}",
    color="darkgray",
    zorder=2,
)

ax.scatter(
    energy_bins_centers,
    ar_sum_sim,
    label=f"Sim G4 {plots_basics.G4_VERSION_NAME}",
    color="purple",
    s=5,
    zorder=3,
)

ax.set_xscale("log")
ax.set_yscale("log")
ax.legend(ncol=2)
plots_basics.style_axes(
    ax,
    "Neutron energy [MeV]",
    "Total cross section [barn]",
    "(n,tot) cross-section in natural Ar - summed cross-sections",
)
plots_basics.save(fig, "neutron_argon_summary.output.png")


fig, ax = plt.subplots()
ax.plot(
    energy_bins_centers,
    jeff_3_cross_sections_36,
    label="JEFF 3.3",
    color="black",
    zorder=1,
)

ax.plot(
    energy_bins_centers,
    sig_36_sum_binned,
    label=f"{latest_g4ndl} {msg_36}",
    color="darkgray",
    zorder=2,
)

ax.scatter(
    energy_bins_centers,
    ar_36_sim,
    label=f"Sim G4 {plots_basics.G4_VERSION_NAME}",
    color="purple",
    s=5,
    zorder=3,
)

ax.set_xscale("log")
ax.set_yscale("log")
ax.legend(ncol=2)
plots_basics.style_axes(
    ax,
    "Neutron energy [MeV]",
    "Total cross section [barn]",
    "(n,tot) cross-section in 36-Ar",
)
plots_basics.save(fig, "neutron_argon36_comparison.output.png")

fig, ax = plt.subplots()
ax.plot(
    energy_bins_centers,
    jeff_3_cross_sections_38,
    label="JEFF 3.3",
    color="black",
    zorder=1,
)

ax.plot(
    energy_bins_centers,
    sig_38_sum_binned,
    label=f"{latest_g4ndl} {msg_38}",
    color="darkgray",
    zorder=2,
)

ax.scatter(
    energy_bins_centers,
    ar_38_sim,
    label=f"Sim G4 {plots_basics.G4_VERSION_NAME}",
    color="purple",
    s=5,
    zorder=3,
)

ax.set_xscale("log")
ax.set_yscale("log")
ax.legend(ncol=2)
plots_basics.style_axes(
    ax,
    "Neutron energy [MeV]",
    "Total cross section [barn]",
    "(n,tot) cross-section in 38-Ar",
)
plots_basics.save(fig, "neutron_argon38_comparison.output.png")

fig, ax = plt.subplots()
ax.plot(
    energy_bins_centers,
    jeff_3_cross_sections_40,
    label="JEFF 3.3",
    color="black",
    zorder=1,
)

ax.plot(
    energy_bins_centers,
    sig_40_sum_binned,
    label=f"{latest_g4ndl} {msg_40}",
    color="darkgray",
    zorder=2,
)

ax.scatter(
    energy_bins_centers,
    ar_40_sim,
    label=f"Sim G4 {plots_basics.G4_VERSION_NAME}",
    color="purple",
    s=5,
    zorder=3,
)

ax.set_xscale("log")
ax.set_yscale("log")
ax.legend(ncol=2)
plots_basics.style_axes(
    ax,
    "Neutron energy [MeV]",
    "Total cross section [barn]",
    "(n,tot) cross-section in 40-Ar",
)
plots_basics.save(fig, "neutron_argon40_comparison.output.png")
