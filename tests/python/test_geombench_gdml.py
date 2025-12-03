from __future__ import annotations

from pathlib import Path

from remage.geombench.gdml_handling import (
    extract_component_from_gdml,
    generate_tmp_gdml_geometry,
    load_gdml_geometry,
)


def test_extract_component_from_gdml() -> None:
    geometry_path = "gdml/geometry.gdml"
    component_name = "germanium"

    loaded_gdml = load_gdml_geometry(geometry_path)

    extracted_gdml = extract_component_from_gdml(loaded_gdml, lv_name=component_name)

    assert component_name in extracted_gdml["registry"].logicalVolumeDict
    assert extracted_gdml["object_lv"].name == component_name


def test_generate_tmp_gdml_geometry() -> None:
    geometry_path = "gdml/geometry.gdml"
    component_name = "germanium"

    loaded_gdml = load_gdml_geometry(geometry_path)

    generated_file_path = generate_tmp_gdml_geometry(loaded_gdml)

    with Path(generated_file_path).open("r") as f:
        gdml_content = f.read()

    assert f'name="{component_name}"' in gdml_content
