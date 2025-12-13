from __future__ import annotations

from pathlib import Path

from remage.geombench.gdml_handling import (
    generate_tmp_gdml_geometry,
    load_gdml_geometry,
)


def test_generate_tmp_gdml_geometry() -> None:
    geometry_path = "gdml/geometry.gdml"
    component_name = "germanium"

    loaded_gdml = load_gdml_geometry(geometry_path)

    generated_file_path = generate_tmp_gdml_geometry(loaded_gdml)

    with Path(generated_file_path).open("r") as f:
        gdml_content = f.read()

    assert f'name="{component_name}"' in gdml_content
