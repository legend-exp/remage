from __future__ import annotations

from pathlib import Path

import awkward as ak
import matplotlib.pyplot as plt
import numpy as np
import pyg4ometry
import vtk
from lgdo import lh5
from matplotlib.colors import LogNorm, Normalize



class SummaryGenerator:
    """
    Class to generate summary analysis for remage geometry benchmark outputs.
    """

    def __init__(self, sim_output_file, args):
        self.data_xy = lh5.read("benchmark_xy", sim_output_file).view_as("ak")
        self.data_xz = lh5.read("benchmark_xz", sim_output_file).view_as("ak")
        self.data_yz = lh5.read("benchmark_yz", sim_output_file).view_as("ak")

        self.n_x_gridpoint = len(np.unique(self.data_xy["X"]))
        self.n_y_gridpoint = len(np.unique(self.data_xy["Y"]))
        self.n_z_gridpoint = len(np.unique(self.data_xz["Z"]))

        self.x = np.linspace(
            np.min(self.data_xy["X"]),
            np.max(self.data_xy["X"]) + (self.data_xy["X"][1] - self.data_xy["X"][0]),
            self.n_x_gridpoint + 1,
        )
        self.y = np.linspace(
            np.min(self.data_xy["Y"]),
            np.max(self.data_xy["Y"]) + (self.data_yz["Y"][1] - self.data_yz["Y"][0]),
            self.n_y_gridpoint + 1,
        )
        self.z = np.linspace(
            np.min(self.data_xz["Z"]),
            np.max(self.data_xz["Z"]) + (self.data_xz["Z"][1] - self.data_xz["Z"][0]),
            self.n_z_gridpoint + 1,
        )

        self.gdml_file = Path(args.geometry)
        self.output_file_template = str(self.gdml_file.name).replace(".gdml", "")
        self.output_dir = Path(args.output_dir)

        self.mult_map_3d = None

    def _multiplicative_reconstruction(self):
        """
        Reconstruct 3D using multiplication of normalized projections.
        This assumes independence and that hotspots must appear in all views.
        """
        map_3d = np.ones((self.n_x_gridpoint, self.n_y_gridpoint, self.n_z_gridpoint))

        # Normalize each projection
        xy_norm = np.array(self.data_xy["Time"]).reshape(
            self.n_y_gridpoint, self.n_x_gridpoint
        )
        xy_norm = xy_norm / np.max(xy_norm) if np.max(xy_norm) > 0 else xy_norm

        xz_norm = np.array(self.data_xz["Time"]).reshape(
            self.n_z_gridpoint, self.n_x_gridpoint
        )
        xz_norm = xz_norm / np.max(xz_norm) if np.max(xz_norm) > 0 else xz_norm

        yz_norm = np.array(self.data_yz["Time"]).reshape(
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

    def draw_simulation_time_profiles(self, suffix="simulation_time_profiles.pdf"):
        """
        Draw the simulation times per event for the three projections.
        """

        def draw(self, norm):
            fig, ax = plt.subplots(1, 3, figsize=(15, 5))
            img = ax[0].matshow(
                np.array(self.data_xy["Time"]).reshape(
                    self.n_y_gridpoint, self.n_x_gridpoint
                )[::-1]
                * 1e6,
                extent=[
                    np.min(self.data_xy["X"]),
                    np.max(self.data_xy["X"]),
                    np.min(self.data_xy["Y"]),
                    np.max(self.data_xy["Y"]),
                ],
                norm=norm,
            )
            ax[0].set_title("XY Plane")
            ax[0].set_xlabel("X")
            ax[0].set_ylabel("Y")
            fig.colorbar(
                img,
                ax=ax[0],
                orientation="horizontal",
                label=r"Median sim time per event [$\mu$s]",
            )

            img = ax[1].matshow(
                np.array(self.data_xz["Time"]).reshape(
                    self.n_z_gridpoint, self.n_x_gridpoint
                )[::-1]
                * 1e6,
                extent=[
                    np.min(self.data_xz["X"]),
                    np.max(self.data_xz["X"]),
                    np.min(self.data_xz["Z"]),
                    np.max(self.data_xz["Z"]),
                ],
                norm=norm,
            )
            ax[1].set_title("XZ Plane")
            ax[1].set_xlabel("X")
            ax[1].set_ylabel("Z")
            fig.colorbar(
                img,
                ax=ax[1],
                orientation="horizontal",
                label=r"Median sim time per event [$\mu$s]",
            )

            img = ax[2].matshow(
                np.array(self.data_yz["Time"]).reshape(
                    self.n_z_gridpoint, self.n_y_gridpoint
                )[::-1]
                * 1e6,
                extent=[
                    np.min(self.data_yz["Y"]),
                    np.max(self.data_yz["Y"]),
                    np.min(self.data_yz["Z"]),
                    np.max(self.data_yz["Z"]),
                ],
                norm=norm,
            )
            ax[2].set_title("YZ Plane")
            ax[2].set_xlabel("Y")
            ax[2].set_ylabel("Z")
            fig.colorbar(
                img,
                ax=ax[2],
                orientation="horizontal",
                label=r"Median sim time per event [$\mu$s]",
            )

            norm_string = "lin"
            if isinstance(norm, LogNorm):
                norm_string = "log"

            output_path = (
                self.output_dir / f"{self.output_file_template}_{norm_string}_{suffix}"
            )
            fig.savefig(output_path, bbox_inches="tight", dpi=300)

            plt.close(fig)

        draw(self, norm=LogNorm())
        draw(self, norm=Normalize())

    def draw_multiplicative(self, suffix="multiplicative.pdf"):
        if self.mult_map_3d is None:
            self._multiplicative_reconstruction()

        def draw(self, norm):
            fig, ax = plt.subplots(1, 3, figsize=(15, 5))
            img = ax[0].matshow(
                np.sum(self.mult_map_3d, axis=2)[::-1].T
                / np.max(np.sum(self.mult_map_3d, axis=2)),
                extent=[
                    np.min(self.data_xy["X"]),
                    np.max(self.data_xy["X"]),
                    np.min(self.data_xy["Y"]),
                    np.max(self.data_xy["Y"]),
                ],
                norm=norm,
            )
            ax[0].set_title("XY Plane")
            ax[0].set_xlabel("X")
            ax[0].set_ylabel("Y")
            fig.colorbar(img, ax=ax[0], orientation="horizontal")

            img = ax[1].matshow(
                np.sum(self.mult_map_3d, axis=1)[::-1].T
                / np.max(np.sum(self.mult_map_3d, axis=1)),
                extent=[
                    np.min(self.data_xz["X"]),
                    np.max(self.data_xz["X"]),
                    np.min(self.data_xz["Z"]),
                    np.max(self.data_xz["Z"]),
                ],
                norm=norm,
            )
            ax[1].set_title("XZ Plane")
            ax[1].set_xlabel("X")
            ax[1].set_ylabel("Z")
            fig.colorbar(img, ax=ax[1], orientation="horizontal")

            img = ax[2].matshow(
                np.sum(self.mult_map_3d, axis=0)[::-1].T
                / np.max(np.sum(self.mult_map_3d, axis=0)),
                extent=[
                    np.min(self.data_yz["Y"]),
                    np.max(self.data_yz["Y"]),
                    np.min(self.data_yz["Z"]),
                    np.max(self.data_yz["Z"]),
                ],
                norm=norm,
            )
            ax[2].set_title("YZ Plane")
            ax[2].set_xlabel("Y")
            ax[2].set_ylabel("Z")
            fig.colorbar(img, ax=ax[2], orientation="horizontal")

            norm_string = "lin"
            if isinstance(norm, LogNorm):
                norm_string = "log"

            output_path = (
                self.output_dir / f"{self.output_file_template}_{norm_string}_{suffix}"
            )
            fig.savefig(output_path)

            plt.close(fig)

        draw(self, norm=LogNorm())
        draw(self, norm=Normalize())

    def _get_hotspot_locations(self, threshold=0.8):
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
            hotspot_coords.append((float(x_coord), float(y_coord), float(z_coord)))

        return hotspot_coords

    def calculate_simulation_statistics(
        self, suffix="_stats.yaml", only_non_world_volumes=True
    ):
        """
        Calculate simulation statistics.
        These consist of:
        - simulation time per event:
            - mean
            - std
            - min/max
        - simulation time per event and volume:
            - mean
            - std
            - min/max
        - locations of hotspots, defined by multiplicative reconstruction
        """

        stats = {
            "simulation_time_per_event": {
                "mean": float(np.mean(ak.to_numpy(self.data_xy["Time"]))),
                "std": float(np.std(ak.to_numpy(self.data_xy["Time"]))),
                "min": float(np.min(ak.to_numpy(self.data_xy["Time"]))),
                "max": float(np.max(ak.to_numpy(self.data_xy["Time"]))),
            },
            "hotspots": self._get_hotspot_locations(threshold=0.8),
        }

        output_path = self.output_dir / f"{self.output_file_template}{suffix}"
        import yaml

        with open(output_path, "w") as f:
            yaml.dump(stats, f)

        return stats

    def perform_analysis(self):
        self.draw_simulation_time_profiles()
        self.draw_multiplicative()
        return self.calculate_simulation_statistics()
