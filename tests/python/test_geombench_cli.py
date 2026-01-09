from __future__ import annotations

import re
from pathlib import Path

from lgdo import lh5
from remage.geombench.cli import generate_macro, remage_geombench_cli


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
