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
import ast
from pathlib import Path

from .. import logging as rmg_logging
from ..cli import remage_run
from .gdml_handling import (
    generate_tmp_gdml_geometry,
    load_gdml_geometry,
)
from .summary_generator import SummaryGenerator


def generate_macro(args, output_file_stem: str = "") -> str:
    """Generate a macro file for the remage geometry benchmark.

    This function creates a macro file based on the provided command-line
    arguments, which will be used to configure the remage simulation for the
    geometry benchmark.

    Parameters
    ----------
    args
        Parsed command-line arguments.

    """

    geometry_path = Path(args.geometry)
    filename = geometry_path.absolute()
    output_dir = Path(args.output_dir)
    output_file = output_dir / (output_file_stem + ".lh5")
    if issubclass(type(args.grid_increments), str) and args.grid_increments:
        grid_increments = ast.literal_eval(args.grid_increments)
        increment_x = grid_increments.get("x", args.grid_increment)
        increment_y = grid_increments.get("y", args.grid_increment)
        increment_z = grid_increments.get("z", args.grid_increment)
    else:
        increment_x = args.grid_increment
        increment_y = args.grid_increment
        increment_z = args.grid_increment
    n_events = args.num_events

    commands = [
        f"/RMG/Geometry/IncludeGDMLFile {filename}",
        "/RMG/Output/ActivateOutputScheme GeomBench",
        f"/RMG/Output/FileName {output_file}",
        "/run/initialize",
        "/RMG/Generator/Select GeomBench",
        f"/RMG/Generator/Benchmark/IncrementX {increment_x} mm",
        f"/RMG/Generator/Benchmark/IncrementY {increment_y} mm",
        f"/RMG/Generator/Benchmark/IncrementZ {increment_z} mm",
        f"/run/beamOn {n_events}",
    ]
    macro_content = "\n".join(commands)

    if args.dry_run:
        logger = rmg_logging.setup_log()
        logger.info("Dry run enabled. Generated macro content:")
        logger.info(macro_content)
        return ""

    return macro_content


def remage_geombench_cli(external_args: list[str] | None = None) -> int:
    """Command-line interface for the remage geometry benchmark.

    This function parses command-line arguments, sets up the macros, runs
    the geometry benchmark using the remage simulation framework, and analyzes
    the output plus provides a summary of the results.

    Returns
    -------
    int
        Exit code of the program. Returns 0 on success, non-zero on failure.
    """
    parser = argparse.ArgumentParser(
        description="Run remage geometry benchmark and analyze results.",
        allow_abbrev=False,
    )
    parser.add_argument(
        "geometry",
        type=str,
        help="Path to the geometry file to be used in the benchmark.",
    )
    parser.add_argument(
        "--logical-volume",
        type=str,
        default="",
        help="In case one is interested in a specific logical volume in a complex GDML geometry, only this volume will be benchmarked.",
    )
    parser.add_argument(
        "--buffer-fraction",
        type=float,
        default=0.25,
        help="Fractional buffer to add around the geometry. For example, 0.25 adds 12.5%% extra space on each side.",
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default="./",
        help="Directory to store benchmark output files.",
    )
    parser.add_argument(
        "--num-events",
        type=int,
        default=10000000,
        help="Number of events to simulate in the benchmark.",
    )
    parser.add_argument(
        "--grid-increment",
        type=float,
        default=1,
        help="Increment between grid points in the benchmark geometry given in mm. The same for all dimensions.",
    )
    parser.add_argument(
        "--grid-increments",
        type=str,
        default="",
        help="Increment of individual grid point distances per dimension given in mm. Example: \"{'x': 1., 'y': 2., 'z': 3}\"",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="If set, only generate the macro file without running remage.",
    )

    # merge external args if provided
    args = parser.parse_args(args=external_args)

    logger = rmg_logging.setup_log()

    tmp_gdml_file = ""

    original_gdml_dict = load_gdml_geometry(Path(args.geometry))
    output_file_stem = Path(args.geometry).stem

    # Extract specific component if requested, otherwise use full geometry
    if args.logical_volume != "":
        registry = original_gdml_dict["registry"]
        if args.logical_volume not in registry.logicalVolumeDict:
            msg = f"Logical volume '{args.logical_volume}' not found in the geometry registry."
            raise ValueError(msg)

        geometry_to_benchmark = {
            "object_lv": registry.logicalVolumeDict[args.logical_volume],
            "registry": registry,
        }

        object_name = f"{args.logical_volume}_extracted"
        output_file_stem = f"part_{args.logical_volume}_{output_file_stem}"
    else:
        geometry_to_benchmark = original_gdml_dict
        object_name = "object_lv"

    # Generate temporary GDML with buffered world volume
    tmp_gdml_file = generate_tmp_gdml_geometry(
        geometry_to_benchmark,
        buffer_fraction=args.buffer_fraction,
        object_name=object_name,
    )
    args.geometry = str(tmp_gdml_file)

    macro_content = generate_macro(args, output_file_stem=output_file_stem)

    if args.dry_run:
        return 0

    try:
        # run remage
        ec, _ = remage_run(macros=macro_content)

        sim_output_file = Path(args.output_dir) / (output_file_stem + ".lh5")

        if ec != 0 and not sim_output_file.exists():
            logger.error("Remage simulation failed.")
            return int(ec)

        sum_gen = SummaryGenerator(
            sim_output_file=sim_output_file,
            args=args,
            output_file_stem=output_file_stem,
        )
        analysis_results = sum_gen.perform_analysis()
        logger.info("Geometry Benchmark Analysis Results:")
        for key, value in analysis_results.items():
            msg = f"{key}: {value}"
            logger.info(msg)

    finally:
        if tmp_gdml_file:
            Path(tmp_gdml_file).unlink(missing_ok=True)

    return 0
