from __future__ import annotations

import argparse
import re
from pathlib import Path

import lh5
import numpy as np
from remage.geombench.cli import generate_macro, remage_geombench_cli
from remage.geombench.summary_generator import SummaryGenerator


def test_remage_geombench_cli(tmp_path: Path) -> None:
    geometry_path = "gdml/geometry.gdml"

    output_dir = tmp_path / "output"
    output_dir.mkdir()

    args = [
        geometry_path,
        "--output-dir",
        str(output_dir),
        "--num-events",
        "100000",
        "--grid-increment",
        "1",
    ]

    remage_geombench_cli(external_args=args)

    assert (output_dir / "geometry.lh5").exists()
    assert (output_dir / "geometry_lin_simulation_time_profiles.pdf").exists()
    assert (output_dir / "geometry_stats.yaml").exists()

    # check if data was written correctly
    assert lh5.read("benchmark_xy", output_dir / "geometry.lh5") is not None


def test_remage_geombench_cli_unequal_axes(tmp_path: Path) -> None:
    output_dir = tmp_path / "output"
    output_dir.mkdir()

    args = [
        "gdml/geometry.gdml",
        "--output-dir",
        str(output_dir),
        "--num-events",
        "20000",
        "--grid-increments",
        "{'x': 20, 'y': 15, 'z': 10}",
    ]

    remage_geombench_cli(external_args=args)

    assert (output_dir / "geometry_lin_simulation_time_profiles.pdf").exists()
    assert (output_dir / "geometry_log_simulation_time_profiles.pdf").exists()
    assert (output_dir / "geometry_stats.yaml").exists()

    gen = SummaryGenerator(
        output_dir / "geometry.lh5",
        argparse.Namespace(geometry="gdml/geometry.gdml", output_dir=output_dir),
        "geometry",
    )
    assert len({gen.n_x_gridpoint, gen.n_y_gridpoint, gen.n_z_gridpoint}) == 3

    # bin edges must span the same distance as the pixel increments
    for edges, step in ((gen.x, 20), (gen.y, 15), (gen.z, 10)):
        assert np.allclose(np.diff(edges), step)


def test_generate_macro(tmp_path: Path) -> None:
    class Args:
        geometry = Path("gdml/geometry.gdml")
        output_dir = tmp_path / "output"
        num_events = 1000000
        grid_increment = 2
        grid_increments = ""
        dry_run = False

    args = Args()

    output_file_stem = args.geometry.stem

    macro_content = generate_macro(args, output_file_stem=output_file_stem)

    assert re.search(
        r"/RMG/Geometry/IncludeGDMLFile .*/gdml/geometry.gdml", macro_content
    )
    assert re.search(r"/RMG/Output/FileName .*/output/geometry.lh5", macro_content)
    assert re.search(r"/RMG/Generator/Benchmark/IncrementX 2", macro_content)
    assert re.search(r"/run/beamOn 1000000", macro_content)
