from __future__ import annotations

from collections import Counter

import awkward as ak
import lh5
import matplotlib.pyplot as plt
import numpy as np
import plots_basics


# Due to the way the Trackoutput works this needs the pdg code of the isotope that is created after capture.
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


def latex_isotope(label):
    element, A = label.split("-")
    return rf"$^{{{A}}}$" + element


def make_big_spectrum(data, flag, ordered_isotopes, xmax, xtext_offset):

    bins = np.linspace(0, xmax, 400)

    _, ax = plt.subplots(figsize=(10, 6))

    offset_factor = 1e5
    current_offset = 1.0

    colors = [
        "red",
        "green",
        "turquoise",
        "navy",
        "olive",
        "purple",
        "black",
    ]

    for i, isotope in enumerate(reversed(ordered_isotopes)):
        g_norm, _ = gamma_spectrum_and_sum(data, isotope)

        counts, _ = np.histogram(np.asarray(g_norm), bins=bins)
        counts = counts.astype(float)
        counts[counts == 0] = 0.1

        counts_offset = counts * current_offset

        iso_label = plots_basics.pdg_to_isotope(isotope)

        ax.stairs(
            counts_offset,
            bins * 1000,
            linewidth=1.3,
            color=colors[i],
        )

        # place isotope label on right
        ax.text(
            xmax * 1000 - xtext_offset,  # x-position (keV)
            current_offset * 10,  # y-position
            latex_isotope(iso_label),
            color=colors[i],
            fontsize=22,
            va="center",
        )

        current_offset *= offset_factor

    # axis styling
    ax.set_yscale("log")
    ax.set_xlim(0, xmax * 1000)
    plots_basics.style_axes(
        ax,
        "Photon energy [keV]",
        "Counts",
        f"nCapture - {flag} (Geant4 {plots_basics.G4_VERSION_NAME})",
    )

    plots_basics.save(
        ax.figure, f"neutron_spectrum_{flag.lower().replace(' ', '_')}.output.png"
    )


def make_pie_chart(data, flag):
    # Convert PDG codes to isotope names
    particles = ak.to_numpy(data.particle[data.particle != 22])

    # Sort to have a consistent order in the pie chart across datasets.
    isotopes = sorted(plots_basics.pdg_to_isotope(p) for p in particles)

    # Count
    counts = Counter(isotopes)

    # Group small contributions
    threshold = 0.02
    total = sum(counts.values())

    large = {}
    small_sum = 0

    for iso, cnt in counts.items():
        if cnt / total < threshold:
            small_sum += cnt
        else:
            large[iso] = cnt

    if small_sum > 0:
        large["Other"] = small_sum

    counts = large

    # Plot
    labels = list(counts.keys())
    sizes = list(counts.values())

    colors = [
        plots_basics.COLOR_CYCLE[i % len(plots_basics.COLOR_CYCLE)]
        for i in range(len(labels))
    ]

    fig, ax = plt.subplots(figsize=(8, 8))
    ax.pie(
        sizes,
        labels=labels,
        autopct="%1.1f%%",
        startangle=90,
        colors=colors,
        textprops={"fontsize": 12},
    )
    ax.set_title(
        f"Isotope Distribution - {flag} (Geant4 {plots_basics.G4_VERSION_NAME})"
    )

    plots_basics.save(
        fig, f"neutron_pie_chart_{flag.lower().replace(' ', '_')}.output.png"
    )


def double_individual_spectrum(data_evap, data_norm, isotope):
    g_norm, s_norm = gamma_spectrum_and_sum(data_norm, isotope)
    g_evap, s_evap = gamma_spectrum_and_sum(data_evap, isotope)

    fig, (ax_norm, ax_evap) = plt.subplots(1, 2, sharey=True)

    color1, color2 = plots_basics.COLOR_CYCLE[:2]

    bins = np.linspace(0, 10.6, 250)  # MeV

    ax_norm.hist(g_norm, bins=bins, histtype="step", color=color1, label="y spectrum")
    ax_norm.hist(
        s_norm, bins=bins, histtype="step", linestyle="--", color=color2, label="ΣEy"
    )

    ax_evap.hist(g_evap, bins=bins, histtype="step", color=color1)
    ax_evap.hist(s_evap, bins=bins, histtype="step", linestyle="--", color=color2)

    plots_basics.style_axes(
        ax_norm, "Energy [MeV]", "Counts", f"Shielding ({len(s_norm)})"
    )
    plots_basics.style_axes(
        ax_evap, "Energy [MeV]", "", f"G4PhotonEvaporation ({len(s_evap)})"
    )

    expected = expected_sum_energy.get(int(isotope))
    for ax in (ax_norm, ax_evap):
        ax.axvline(expected, color="black", linestyle=":", label="Expected ΣEy")
        ax.set_yscale("log")
        ax.legend()

    fig.suptitle(
        f"Neutron capture on {latex_isotope(plots_basics.pdg_to_isotope(isotope))} (Geant4 {plots_basics.G4_VERSION_NAME})"
    )

    ax_norm.set_yscale("log")
    plots_basics.save(
        fig,
        f"neutron_capture_{plots_basics.pdg_to_isotope(isotope).replace('-', '_')}.output.png",
    )


# ----------------------------------------

# Expected summed gamma energies [MeV] keyed by PDG code
# We need to key this with the code of the isotope after capture, which is what Geant4 gives us in the track output.
expected_sum_energy = {
    1000010020: 2.2224,  # Capture on H-1 produces H-2
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

# Plots for the individual gamma lines and the summed energy comparing shielding vs evap
data_evap = lh5.read_as("/tracks", "neutron_captures_evap.lh5", "ak")
data_shielding = lh5.read_as("/tracks", "neutron_captures_shielding.lh5", "ak")

for isotope in expected_sum_energy:
    double_individual_spectrum(data_evap, data_shielding, isotope)

# Do the one big spectrum for the steel isotopes
# Again these are after capture due to the Geant4 output.
# Will be converted to before capture labels in the plotting.
ordered_isotopes = [
    1000240540,
    1000260550,
    1000240510,
    1000280590,
    1000240530,
    1000280610,
    1000260570,
]
data = lh5.read_as("/tracks", "neutron_captures_steel.lh5", "ak")
make_big_spectrum(
    data, "steel Shielding", ordered_isotopes, xmax=10.6, xtext_offset=800
)

# Make a bunch of pie charts to show the isotope distribution after capture for the different datasets.
make_pie_chart(data, "steel")
make_pie_chart(data_shielding, "general")

data_normal = lh5.read_as("/tracks", "neutron_captures_normal.lh5", "ak")
data_rmgncapture = lh5.read_as("/tracks", "neutron_captures_rmgncapture.lh5", "ak")

make_pie_chart(data_normal, "normal")
make_pie_chart(data_rmgncapture, "RMGnCapture")

# Also do the big spectrum for gadolinium
ordered_isotopes_gd = [
    1000641560,
    1000641580,
]
make_big_spectrum(
    data_normal, "gd Shielding", ordered_isotopes_gd, xmax=9.6, xtext_offset=1100
)
