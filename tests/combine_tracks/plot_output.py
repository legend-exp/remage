from __future__ import annotations

from pathlib import Path

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
from lgdo import lh5


def analyze_tracks(energy: str):
    # First get file sizes, energy sums, and unique track counts
    combined_file = f"combined_{energy}.lh5"
    uncombined_file = f"uncombined_{energy}.lh5"

    combined_size = Path(combined_file).stat().st_size / (1024 * 1024)
    uncombined_size = Path(uncombined_file).stat().st_size / (1024 * 1024)

    data_combined = lh5.read_as("/stp/det001", combined_file, "ak")
    data_uncombined = lh5.read_as("/stp/det001", uncombined_file, "ak")
    unique_tracks_combined = sum(len(np.unique(evt)) for evt in data_combined.trackid)
    unique_tracks_uncombined = sum(
        len(np.unique(evt)) for evt in data_uncombined.trackid
    )

    # Total energy per event
    total_energy_combined = ak.sum(data_combined.edep, axis=1)
    total_energy_uncombined = ak.sum(data_uncombined.edep, axis=1)

    assert np.allclose(
        ak.to_numpy(total_energy_combined),
        ak.to_numpy(total_energy_uncombined),
        rtol=1e-12,
        atol=1e-9,
    )

    # Now make plots of energy deposition vs location
    n_bins = 50
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))

    for idx, (data, fsize, n_tracks) in enumerate(
        zip(
            [data_combined, data_uncombined],
            [combined_size, uncombined_size],
            [unique_tracks_combined, unique_tracks_uncombined],
            strict=True,
        )
    ):
        xloc = ak.flatten(data.xloc)
        yloc = ak.flatten(data.yloc)
        zloc = ak.flatten(data.zloc)
        energies = ak.flatten(data.edep)

        for idy, loc in enumerate([xloc, yloc, zloc]):
            binning = np.linspace(np.min(loc), np.max(loc), n_bins + 1)
            energy_sum, _ = np.histogram(loc, bins=binning, weights=energies)
            bin_centers = 0.5 * (binning[:-1] + binning[1:])

            axes[idx].plot(
                bin_centers,
                energy_sum,
                drawstyle="steps-mid",
                label=f"{['x', 'y', 'z'][idy]}-location",
            )

        axes[idx].set_xlabel("world location (m)")
        axes[idx].set_ylabel("Total Energy Deposited (keV)")
        axes[idx].set_title(
            f"{'Combined' if idx == 0 else 'Uncombined'} - Energy deposition vs location {energy} keV primary"
        )
        axes[idx].legend()

        axes[idx].text(
            0.95,
            0.95,
            f"File size: {fsize:.2f} MB\nUnique Tracks: {n_tracks}",
            transform=axes[idx].transAxes,
            fontsize=10,
            verticalalignment="top",
            horizontalalignment="right",
            bbox={"boxstyle": "round,pad=0.3", "facecolor": "white", "alpha": 0.7},
        )

    fig.savefig(f"combine_tracks_edep_locs_{energy}keV.png")


for energy in ["500", "2000", "5000"]:
    analyze_tracks(energy)
