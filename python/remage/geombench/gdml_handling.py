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

import tempfile
from pathlib import Path

import pyg4ometry


def load_gdml_geometry(gdml_path: Path, object_name: str = "object_lv") -> dict:
    """Load a GDML geometry file and rename the world volume.

    Parameters
    ----------
    gdml_path :
        Path to the GDML file.
    object_name : optional
        Name to assign to the loaded world volume. Default is "object_lv".

    """
    reader = pyg4ometry.gdml.Reader(gdml_path)
    registry = reader.getRegistry()
    world_volume = registry.getWorldVolume()

    # Rename the world volume
    original_name = world_volume.name
    world_volume.name = object_name

    # Update registry references
    registry.logicalVolumeDict.pop(original_name, None)
    registry.logicalVolumeList.remove(original_name)
    registry.logicalVolumeDict[object_name] = world_volume
    registry.logicalVolumeList.append(object_name)

    # Update world reference in registry
    registry.setWorld(object_name)

    return {"object_lv": world_volume, "registry": registry}


def change_extent_of_world_volume(
    geometry: dict, buffer_fraction: float = 0, object_name: str = "object_lv"
) -> dict:
    """Expand the world volume to include buffer space around the geometry."""

    world_lv = geometry["object_lv"]
    registry = geometry["registry"]

    # Rename world volume if needed
    if world_lv.name != object_name:
        old_name = world_lv.name
        world_lv.name = object_name

        # Update registry references
        if old_name in registry.logicalVolumeDict:
            registry.logicalVolumeDict.pop(old_name)
        if old_name in registry.logicalVolumeList:
            registry.logicalVolumeList.remove(old_name)

        registry.logicalVolumeDict[object_name] = world_lv
        if object_name not in registry.logicalVolumeList:
            registry.logicalVolumeList.append(object_name)

    # Always ensure world is set correctly
    registry.setWorld(world_lv.name)

    # Calculate extent and create larger world box
    extent = world_lv.extent(True)
    width = (extent[1][0] - extent[0][0]) * (1 + buffer_fraction)
    height = (extent[1][1] - extent[0][1]) * (1 + buffer_fraction)
    depth = (extent[1][2] - extent[0][2]) * (1 + buffer_fraction)

    # Remove old solid from registry and create new larger box
    old_solid_name = world_lv.solid.name
    if old_solid_name in registry.solidDict:
        del registry.solidDict[old_solid_name]

    new_box = pyg4ometry.geant4.solid.Box(
        old_solid_name, width, height, depth, registry, "mm"
    )

    # Update the logical volume to use the new solid
    world_lv.solid = new_box

    return {"object_lv": world_lv, "registry": registry}


def generate_tmp_gdml_geometry(
    geometry: dict, buffer_fraction: float = 0.25, object_name: str = "object_lv"
) -> Path:
    """Prepare a GDML geometry by wrapping it in a buffered world volume.

    Creates a new GDML file with the geometry positioned in a world volume
    that includes buffer space around it.

    Parameters
    ----------
    geometry :
        Dictionary containing the loaded geometry with keys "object_lv" and "registry".
    buffer_fraction : optional
        Fractional buffer to add around the geometry. For example, 0.25 adds
        12.5%% extra space on each side. Default is 0.25.
    object_name :  optional
        Name to assign to the object logical volume when positioning.
        Default is "object_lv".

    Returns
    -------
    Path
        Path to the temporary GDML file with the adjusted geometry.
    """
    positioned_geometry = change_extent_of_world_volume(
        geometry, buffer_fraction=buffer_fraction, object_name=object_name
    )

    writer = pyg4ometry.gdml.Writer()
    writer.addDetector(positioned_geometry["registry"])

    temp_file = tempfile.NamedTemporaryFile(mode="w", delete=False, suffix=".gdml")  # noqa: SIM115
    tempfile_path = Path(temp_file.name)
    writer.write(tempfile_path)

    return tempfile_path
