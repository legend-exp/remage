from __future__ import annotations

import sys

import numpy as np

# the helpers are shared with the observables-ge test, the file is copied into
# this directory by CMake
from plot_observables import plot, plot_timing

# keep in sync with run_sim.py
all_cluster_distances = [10, 20, 50, 100, 200, 500, 1000, None]  # um
all_cluster_distances_surface = [1, 5, 10, 20, 50, 100, None]  # um
all_cluster_times = [0.1, 10, 1000, None]  # us

# `None` is the run without any pre-clustering, shown as a dashed reference
NO_CLUSTER_LABEL = "No pre-clustering"

plot_name = sys.argv[1]

scans = (
    (
        "cluster_distance",
        all_cluster_distances,
        "um",
        "Pre-cluster distance (bulk)",
        "",
    ),
    (
        "cluster_distance_surface",
        all_cluster_distances_surface,
        "um",
        "Pre-cluster distance (surface)",
        ".surf-dist",
    ),
    ("cluster_time", all_cluster_times, "us", "Pre-cluster time threshold", ".time"),
)

for name, values, unit, title, suffix in scans:
    xlabel = f"{title} [{'µm' if unit == 'um' else 'µs'}]"
    # this is the interesting plot: CPU time and output size vs. the setting
    plot_timing(
        ["beta_bulk", "beta_surf"],
        name,
        values,
        unit,
        save_name=f"{plot_name}.timing{suffix}.output.png",
        xlabel=xlabel,
        default_label=NO_CLUSTER_LABEL,
    )

    common = {
        "values": values,
        "names": [name],
        "unit": unit,
        "xlabel": xlabel,
        "default_label": NO_CLUSTER_LABEL,
        "legend_title": title,
        "doeff": True,
    }

    # the observables, to check that the pre-clustering does not bias them
    plot(
        "beta_bulk",
        (-1, 1020),
        fields=["truth_energy"],
        save_spec_name=f"{plot_name}.bulk-total-energy{suffix}.spec.output.png",
        save_eff_name=f"{plot_name}.bulk-total-energy{suffix}.eff.output.png",
        **common,
    )

    plot(
        "beta_bulk",
        (-1, 1020),
        fields=["active_energy_avg"],
        save_spec_name=f"{plot_name}.bulk-active-energy{suffix}.spec.output.png",
        save_eff_name=f"{plot_name}.bulk-active-energy{suffix}.eff.output.png",
        **common,
    )

    # events with a vertex in the transition layer are the most sensitive ones
    plot(
        "beta_bulk",
        (-1, 1020),
        dist_range=(0, 1),
        fields=["active_energy_avg"],
        save_spec_name=f"{plot_name}.tl-active-energy{suffix}.spec.output.png",
        save_eff_name=f"{plot_name}.tl-active-energy{suffix}.eff.output.png",
        **common,
    )

    plot(
        "beta_bulk",
        (0, 2),
        eff_range=(1, np.inf),
        fields=["r90_avg"],
        n_bins=200,
        range_zoom=None,
        label="r90 [mm]",
        save_spec_name=f"{plot_name}.bulk-r90{suffix}.spec.output.png",
        save_eff_name=f"{plot_name}.bulk-r90{suffix}.eff.output.png",
        **common,
    )

    # plots for the surface
    plot(
        "beta_surf",
        (-1, 1020),
        fields=["active_energy_avg"],
        range_zoom=None,
        eff_range=(300, np.inf),
        save_spec_name=f"{plot_name}.surf-active-energy{suffix}.spec.output.png",
        save_eff_name=f"{plot_name}.surf-active-energy{suffix}.eff.output.png",
        **common,
    )

    plot(
        "beta_surf",
        (0, 2),
        eff_range=(1, np.inf),
        fields=["max_z_avg"],
        n_bins=200,
        range_zoom=None,
        label="Range [mm]",
        save_spec_name=f"{plot_name}.surf-max-z{suffix}.spec.output.png",
        save_eff_name=f"{plot_name}.surf-max-z{suffix}.eff.output.png",
        **common,
    )
