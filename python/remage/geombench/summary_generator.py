# Copyright (C) 2025 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


from __future__ import annotations

import argparse
from pathlib import Path

import awkward as ak
import hist
import matplotlib.pyplot as plt
import numpy as np
import yaml
from lgdo import lh5
from matplotlib.colors import LogNorm, Normalize


class SummaryGenerator:
    """
    Class to generate summary analysis for remage geometry benchmark outputs.
    """

    def __init__(
        self, sim_output_file: Path, args: argparse.Namespace, output_file_stem: str
    ) -> None:
        self.data_xy = lh5.read("benchmark_xy", sim_output_file).view_as("ak")
        self.data_xz = lh5.read("benchmark_xz", sim_output_file).view_as("ak")
        self.data_yz = lh5.read("benchmark_yz", sim_output_file).view_as("ak")

        self.n_x_gridpoint = len(np.unique(self.data_xy["x"]))
        self.n_y_gridpoint = len(np.unique(self.data_xy["y"]))
        self.n_z_gridpoint = len(np.unique(self.data_xz["z"]))

        self.x = np.linspace(
            np.min(self.data_xy["x"]),
            np.max(self.data_xy["x"]) + (self.data_xy["x"][1] - self.data_xy["x"][0]),
            self.n_x_gridpoint + 1,
        )
        self.y = np.linspace(
            np.min(self.data_xy["y"]),
            np.max(self.data_xy["y"]) + (self.data_yz["y"][1] - self.data_yz["y"][0]),
            self.n_y_gridpoint + 1,
        )
        self.z = np.linspace(
            np.min(self.data_xz["z"]),
            np.max(self.data_xz["z"]) + (self.data_xz["z"][1] - self.data_xz["z"][0]),
            self.n_z_gridpoint + 1,
        )

        self.gdml_file = Path(args.geometry)

        self.output_file_stem = output_file_stem
        self.output_dir = Path(args.output_dir)

        self.mult_map_3d = None

    def _multiplicative_reconstruction(self) -> None:
        """
        Reconstruct 3D using multiplication of normalized projections.
        This assumes independence and that hotspots must appear in all views.
        """
        map_3d = np.ones((self.n_x_gridpoint, self.n_y_gridpoint, self.n_z_gridpoint))

        # Normalize each projection
        xy_norm = np.array(self.data_xy["time"]).reshape(
            self.n_y_gridpoint, self.n_x_gridpoint
        )
        xy_norm = xy_norm / np.max(xy_norm) if np.max(xy_norm) > 0 else xy_norm

        xz_norm = np.array(self.data_xz["time"]).reshape(
            self.n_z_gridpoint, self.n_x_gridpoint
        )
        xz_norm = xz_norm / np.max(xz_norm) if np.max(xz_norm) > 0 else xz_norm

        yz_norm = np.array(self.data_yz["time"]).reshape(
            self.n_z_gridpoint, self.n_y_gridpoint
        )
        yz_norm = yz_norm / np.max(yz_norm) if np.max(yz_norm) > 0 else yz_norm

        # Multiply projections (back-project and multiply)
        for ix in range(self.n_x_gridpoint):
            for iy in range(self.n_y_gridpoint):
                for iz in range(self.n_z_gridpoint):
                    map_3d[ix, iy, iz] = (
                        xy_norm[iy, ix] * xz_norm[iz, ix] * yz_norm[iz, iy]
                    )

        self.mult_map_3d = map_3d

    def draw_simulation_time_profiles(
        self, suffix: str = "simulation_time_profiles.pdf"
    ) -> None:
        """
        Draw the simulation times per event for the three projections.
        """

        def draw(self, norm):
            fig, ax = plt.subplots(1, 3, figsize=(15, 5))
            # No self.fig; pass fig explicitly to helpers

            # Create histogram for XY plane
            h_xy = hist.Hist(
                hist.axis.Regular(
                    self.n_x_gridpoint, self.x[0], self.x[-1], name="x", label="x"
                ),
                hist.axis.Regular(
                    self.n_y_gridpoint, self.y[0], self.y[-1], name="y", label="y"
                ),
            )
            h_xy[:, :] = (
                np.array(self.data_xy["time"])
                .reshape(self.n_x_gridpoint, self.n_y_gridpoint)
                .T
                * 1e6
            )
            artists_xy = h_xy.plot2d(
                ax=ax[0], norm=norm, edgecolor="face", linewidth=0, cbar=False
            )
            ax[0].set_title("XY Plane")
            ax[0].set_box_aspect((self.y[-1] - self.y[0]) / (self.x[-1] - self.x[0]))
            cbar_xy = fig.colorbar(artists_xy[0], ax=ax[0], orientation="horizontal")
            cbar_xy.set_label(r"Median sim time per event [$\mu$s]")

            # Create histogram for XZ plane
            h_xz = hist.Hist(
                hist.axis.Regular(
                    self.n_x_gridpoint, self.x[0], self.x[-1], name="x", label="x"
                ),
                hist.axis.Regular(
                    self.n_z_gridpoint, self.z[0], self.z[-1], name="z", label="z"
                ),
            )
            h_xz[:, :] = (
                np.array(self.data_xz["time"])
                .reshape(self.n_x_gridpoint, self.n_z_gridpoint)
                .T
                * 1e6
            )
            artists_xz = h_xz.plot2d(
                ax=ax[1], norm=norm, edgecolor="face", linewidth=0, cbar=False
            )
            ax[1].set_title("XZ Plane")
            ax[1].set_box_aspect((self.z[-1] - self.z[0]) / (self.x[-1] - self.x[0]))
            cbar_xz = fig.colorbar(artists_xz[0], ax=ax[1], orientation="horizontal")
            cbar_xz.set_label(r"Median sim time per event [$\mu$s]")

            # Create histogram for YZ plane
            h_yz = hist.Hist(
                hist.axis.Regular(
                    self.n_y_gridpoint, self.y[0], self.y[-1], name="y", label="y"
                ),
                hist.axis.Regular(
                    self.n_z_gridpoint, self.z[0], self.z[-1], name="z", label="z"
                ),
            )
            h_yz[:, :] = (
                np.array(self.data_yz["time"])
                .reshape(self.n_y_gridpoint, self.n_z_gridpoint)
                .T
                * 1e6
            )
            artists_yz = h_yz.plot2d(
                ax=ax[2],
                norm=norm,
                edgecolor="face",
                linewidth=0,
                cbar=False,
            )
            ax[2].set_title("YZ Plane")
            ax[2].set_box_aspect((self.z[-1] - self.z[0]) / (self.y[-1] - self.y[0]))
            cbar_yz = fig.colorbar(artists_yz[0], ax=ax[2], orientation="horizontal")
            cbar_yz.set_label(r"Median sim time per event [$\mu$s]")

            norm_string = "lin"
            if isinstance(norm, LogNorm):
                norm_string = "log"

            output_path = (
                self.output_dir / f"{self.output_file_stem}_{norm_string}_{suffix}"
            )
            fig.savefig(output_path, bbox_inches="tight", dpi=300)

            plt.close(fig)

        draw(self, norm=LogNorm())
        draw(self, norm=Normalize())

    def _get_hotspot_locations(
        self, threshold: float = 0.8
    ) -> list[tuple[float, float, float]]:
        """
        Get hotspot locations from multiplicative reconstruction above a certain threshold.
        """
        if self.mult_map_3d is None:
            self._multiplicative_reconstruction()

        hotspots = np.argwhere(self.mult_map_3d >= threshold * np.max(self.mult_map_3d))
        hotspot_coords = []
        for hs in hotspots:
            x_coord = self.x[hs[0]]
            y_coord = self.y[hs[1]]
            z_coord = self.z[hs[2]]
            hotspot_coords.append([float(x_coord), float(y_coord), float(z_coord)])

        return hotspot_coords

    def calculate_simulation_statistics(self, suffix: str = "_stats.yaml") -> dict:
        """Calculate simulation statistics.

        Args:
            suffix (str): Suffix for the output statistics file.
        Returns:
            dict: Dictionary containing simulation statistics.

        Details:
            - Mean, standard deviation, minimum, and maximum simulation time per event.
            - Hotspot locations identified from the multiplicative reconstruction.

        """

        stats = {
            "simulation_time_per_event": {
                "mean": float(np.mean(ak.to_numpy(self.data_xy["time"]))),
                "std": float(np.std(ak.to_numpy(self.data_xy["time"]))),
                "min": float(np.min(ak.to_numpy(self.data_xy["time"]))),
                "max": float(np.max(ak.to_numpy(self.data_xy["time"]))),
            },
            "hotspots": self._get_hotspot_locations(threshold=0.8),
        }

        output_path = self.output_dir / f"{self.output_file_stem}{suffix}"

        with Path(output_path).open("w") as f:
            yaml.dump(stats, f)

        return stats

    def perform_analysis(self) -> dict:
        self.draw_simulation_time_profiles()
        return self.calculate_simulation_statistics()
