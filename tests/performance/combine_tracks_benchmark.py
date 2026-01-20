from __future__ import annotations

import time
from pathlib import Path

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
from lgdo import lh5
from remage import remage_run

modes = ["true", "false"]
energies = np.array([500, 2000, 5000])  # in keV

combined_elapsed_time = []
uncombined_elapsed_time = []

# Run the other three

for energy in energies:
    start = time.time()
    remage_run(
        macros="macros/template.mac",
        gdml_files="gdml/geometry-box.gdml",
        output=f"combined_{energy}.lh5",
        overwrite_output=True,
        macro_substitutions={
            "MODE": "true",
            "ENERGY": energy,
        },
    )
    end = time.time()
    combined_elapsed_time.append(end - start)

    start = time.time()
    remage_run(
        macros="macros/template.mac",
        gdml_files="gdml/geometry-box.gdml",
        output=f"uncombined_{energy}.lh5",
        overwrite_output=True,
        macro_substitutions={
            "MODE": "false",
            "ENERGY": energy,
        },
    )
    end = time.time()
    uncombined_elapsed_time.append(end - start)


combined_sizes = []
uncombined_sizes = []
combined_tracks = []
uncombined_tracks = []

for energy in energies:
    combined_file = f"combined_{energy}.lh5"
    uncombined_file = f"uncombined_{energy}.lh5"

    combined_sizes.append(Path(combined_file).stat().st_size / (1024 * 1024))
    uncombined_sizes.append(Path(uncombined_file).stat().st_size / (1024 * 1024))

    data_combined = lh5.read_as("/stp/det001", combined_file, "ak")
    data_uncombined = lh5.read_as("/stp/det001", uncombined_file, "ak")

    combined_tracks.append(sum(len(np.unique(evt)) for evt in data_combined.trackid))
    uncombined_tracks.append(
        sum(len(np.unique(evt)) for evt in data_uncombined.trackid)
    )

    total_energy_combined = ak.sum(data_combined.edep, axis=1)
    total_energy_uncombined = ak.sum(data_uncombined.edep, axis=1)

    assert np.allclose(
        ak.to_numpy(total_energy_combined),
        ak.to_numpy(total_energy_uncombined),
        rtol=1e-12,
        atol=1e-9,
    )

max_tracks = max(*combined_tracks, *uncombined_tracks)
max_time = max(*combined_elapsed_time, *uncombined_elapsed_time)

combined_tracks_norm = np.array(combined_tracks) * max_time / max_tracks
uncombined_tracks_norm = np.array(uncombined_tracks) * max_time / max_tracks


x = np.arange(len(energies))

bar_w = 0.12
gap = 0.03

# Offsets for the 3 metrics
offset_time = -2 * bar_w - gap
offset_size = 2 * bar_w + gap
offset_tracks = 0

fig, ax_time = plt.subplots()
ax_size = ax_time.twinx()

# ---- Elapsed time (primary axis) ----
ax_time.bar(
    x + offset_time - bar_w / 2,
    combined_elapsed_time,
    width=bar_w,
    label="Combined time",
    color="green",
)
ax_time.bar(
    x + offset_time + bar_w / 2,
    uncombined_elapsed_time,
    width=bar_w,
    label="Uncombined time",
    color="red",
)

# ---- Normalized unique tracks (primary axis) ----
ax_time.bar(
    x + offset_tracks - bar_w / 2,
    combined_tracks_norm,
    width=bar_w,
    label="Combined unique tracks (norm)",
    color="green",
    hatch="//",
)
ax_time.bar(
    x + offset_tracks + bar_w / 2,
    uncombined_tracks_norm,
    width=bar_w,
    label="Uncombined unique tracks (norm)",
    color="red",
    hatch="//",
)

# ---- File size (secondary axis) ----
ax_size.bar(
    x + offset_size - bar_w / 2,
    combined_sizes,
    width=bar_w,
    label="Combined file size",
    color="green",
    hatch="\\",
)
ax_size.bar(
    x + offset_size + bar_w / 2,
    uncombined_sizes,
    width=bar_w,
    label="Uncombined file size",
    color="red",
    hatch="\\",
)

# ---- Axes formatting ----
ax_time.set_xlabel("Primary gamma energy in keV")
ax_time.set_xticks(x)
ax_time.set_xticklabels(energies)

ax_time.set_ylabel("Elapsed time in s / Normalized tracks")
ax_size.set_ylabel("File size in MB")

ax_time.grid(axis="y", alpha=0.4)

# ---- Combined legend ----
h1, l1 = ax_time.get_legend_handles_labels()
h2, l2 = ax_size.get_legend_handles_labels()
ax_time.legend(h1 + h2, l1 + l2, loc="best")

fig.tight_layout()
plt.savefig("combine-tracks-benchmark.output.png")
plt.close()
