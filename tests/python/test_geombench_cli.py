from __future__ import annotations

import re
from pathlib import Path

import lh5
import pyg4ometry
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


def test_remage_geombench_cli_logical_volume_with_border_surface(
    tmp_path: Path,
) -> None:
    reg = pyg4ometry.geant4.Registry()
    mat = pyg4ometry.geant4.MaterialPredefined("G4_Galactic")

    def box(name, size):
        return pyg4ometry.geant4.solid.Box(name, size, size, size, reg, "mm")

    world = pyg4ometry.geant4.LogicalVolume(box("world", 1000), mat, "world", reg)
    reg.setWorld(world)
    outer = pyg4ometry.geant4.LogicalVolume(box("outer", 100), mat, "outer", reg)
    inner = pyg4ometry.geant4.LogicalVolume(box("inner", 200), mat, "inner", reg)
    daughter = pyg4ometry.geant4.LogicalVolume(
        box("daughter", 50), mat, "daughter", reg
    )

    outer_pv = pyg4ometry.geant4.PhysicalVolume(
        [0, 0, 0], [400, 0, 0], outer, "outer", world, reg
    )
    pyg4ometry.geant4.PhysicalVolume([0, 0, 0], [0, 0, 0], inner, "inner", world, reg)
    daughter_pv = pyg4ometry.geant4.PhysicalVolume(
        [0, 0, 0], [0, 0, 0], daughter, "daughter", inner, reg
    )

    optical = pyg4ometry.geant4.solid.OpticalSurface(
        "surf", "polished", "glisur", "dielectric_dielectric", 1.0, reg
    )
    pyg4ometry.geant4.BorderSurface("border", daughter_pv, outer_pv, optical, reg)

    gdml_path = tmp_path / "surfaces.gdml"
    writer = pyg4ometry.gdml.Writer()
    writer.addDetector(reg)
    writer.write(gdml_path)

    output_dir = tmp_path / "output"
    output_dir.mkdir()

    remage_geombench_cli(
        external_args=[
            str(gdml_path),
            "--logical-volume",
            "inner",
            "--output-dir",
            str(output_dir),
            "--num-events",
            "10000",
            "--grid-increment",
            "10",
        ]
    )

    assert (output_dir / "part_inner_surfaces.lh5").exists()
    assert (output_dir / "part_inner_surfaces_stats.yaml").exists()
