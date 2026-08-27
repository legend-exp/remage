from __future__ import annotations

from pathlib import Path

from remage.geombench.gdml_handling import (
    generate_tmp_gdml_geometry,
    load_gdml_geometry,
)


def test_generate_tmp_gdml_geometry() -> None:
    geometry_path = Path("gdml/geometry.gdml")
    component_name = "germanium"

    loaded_gdml = load_gdml_geometry(geometry_path)

    with generate_tmp_gdml_geometry(loaded_gdml) as generated_file_path:
        gdml_content = generated_file_path.read_text()

    assert f'name="{component_name}"' in gdml_content
    # the context manager cleans up after itself
    assert not generated_file_path.exists()
