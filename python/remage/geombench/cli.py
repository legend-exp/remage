from __future__ import annotations

import argparse
import ast
import tempfile
from pathlib import Path

from ..cli import remage_run
from .summary_generator import SummaryGenerator
from .. import logging as rmg_logging


def generate_output_file_path(geometry_path: Path, output_dir: Path) -> Path:
    """Generate the output file path based on the geometry file name.

    Parameters
    ----------
    geometry_path : Path
        Path to the geometry file.
    output_dir : Path
        Directory where the output file will be stored.

    Returns
    -------
    Path
        Full path to the output file with .lh5 extension.
    """
    filename = geometry_path.absolute()
    output_file = output_dir / filename.name.replace(".gdml", ".lh5")
    return output_file


def generate_macro(args) -> str:
    """Generate a macro file for the remage geometry benchmark.

    This function creates a macro file based on the provided command-line
    arguments, which will be used to configure the remage simulation for the
    geometry benchmark.

    Parameters
    ----------
    args
        Parsed command-line arguments.
        Consisting of:
        - geometry: Path to the geometry file.
        - output_dir: Directory to store benchmark output files.
        - num_events: Number of events to simulate.
        - grid_increment: Increment between grid points in mm.
        - grid_increments: Optional dict of increments per dimension in mm.
        - dry_run: If set, only generate the macro file without running remage.

    Returns
    -------
    str
        Path to the generated macro file.
    """

    geometry_path = Path(args.geometry)
    filename = geometry_path.absolute()
    output_dir = Path(args.output_dir)
    output_file = generate_output_file_path(geometry_path, output_dir)
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

    macro_file_path = tempfile.NamedTemporaryFile(mode="w", delete=False, suffix=".mac")
    macro_file_path.write(macro_content)
    macro_file_path.close()
    return str(macro_file_path.name)


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
        description="Run remage geometry benchmark and analyze results."
    )
    parser.add_argument(
        "geometry",
        type=str,
        help="Path to the geometry file to be used in the benchmark.",
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
        help="Increment of individual grid point distances per dimension given in mm. Example: {'x': 1., 'y': 2., 'z': 3}",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="If set, only generate the macro file without running remage.",
    )

    # merge external args if provided
    if external_args is None:
        args = parser.parse_args()
    else:
        args = parser.parse_args(args=external_args)


    logger = rmg_logging.setup_log()
    macro_file = generate_macro(args)

    if args.dry_run:
        return 0

    try:
        # run remage
        ec, _ = remage_run(macros=macro_file)

        sim_output_file = generate_output_file_path(
            Path(args.geometry), Path(args.output_dir)
        )

        if ec != 0 and not sim_output_file.exists():
            logger.error("Remage simulation failed.")
            return int(ec)

        sum_gen = SummaryGenerator(sim_output_file=sim_output_file, args=args)
        analysis_results = sum_gen.perform_analysis()
        logger.info("Geometry Benchmark Analysis Results:")
        for key, value in analysis_results.items():
            logger.info(f"{key}: {value}")

    except Exception as e:
        logger.error(f"An error occurred: {e}")
        return 1
    finally:
        Path(macro_file).unlink(missing_ok=True)

    return 0
