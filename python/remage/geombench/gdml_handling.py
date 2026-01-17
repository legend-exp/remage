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


def copy_referenced_resources(
    lv: pyg4ometry.geant4.LogicalVolume,
    src_reg: pyg4ometry.geant4.Registry,
    dst_reg: pyg4ometry.geant4.Registry,
) -> None:
    """Copy all resources referenced by a logical volume and its hierarchy.

    This function recursively copies materials, solids, positions, rotations,
    and other defines needed by the logical volume and all its daughter volumes.

    Parameters
    ----------
    lv :
        The logical volume whose resources are to be copied.
    src_reg :
        The source registry from which to copy resources.
    dst_reg :
        The destination registry to which resources will be copied.
    """

    def _copy_named_object(
        obj: object,
        src_dict_name: str,
        dst_dict_name: str,
        src_reg: pyg4ometry.geant4.Registry,
        dst_reg: pyg4ometry.geant4.Registry,
    ) -> None:
        """Copy a named object from source registry dict to destination if not present."""
        if not (obj and hasattr(obj, "name")):
            return

        src_dict = getattr(src_reg, src_dict_name, {})
        if obj.name in src_dict and obj.name not in getattr(dst_reg, dst_dict_name, {}):
            if not hasattr(dst_reg, dst_dict_name):
                setattr(dst_reg, dst_dict_name, {})
            getattr(dst_reg, dst_dict_name)[obj.name] = src_dict[obj.name]

    def _copy_material(
        material: pyg4ometry.geant4.Material,
        src_reg: pyg4ometry.geant4.Registry,
        dst_reg: pyg4ometry.geant4.Registry,
    ) -> None:
        """Copy a material and its property dependencies to the destination registry."""
        if not material or material.name in dst_reg.materialDict:
            return

        # Copy material property references
        if hasattr(material, "properties") and material.properties:
            for prop_value in material.properties.values():
                if hasattr(prop_value, "name"):
                    for dict_name in ["matrixDict", "defineDict"]:
                        _copy_named_object(
                            prop_value, dict_name, dict_name, src_reg, dst_reg
                        )

        material.registry = dst_reg
        dst_reg.addMaterial(material)

    def _copy_solid_recursive(
        solid: pyg4ometry.geant4.Solid,
        src_reg: pyg4ometry.geant4.Registry,
        dst_reg: pyg4ometry.geant4.Registry,
    ) -> None:
        """Recursively copy a solid and all its nested components."""
        if solid.name in dst_reg.solidDict:
            return

        solid.registry = dst_reg
        dst_reg.addSolid(solid)

        # Copy position lists (GenericPolycone, Tessellated, etc.)
        for attr in ["pPositions", "vertices"]:
            if hasattr(solid, attr) and getattr(solid, attr):
                for pos in getattr(solid, attr):
                    _copy_named_object(
                        pos, "positionDict", "positionDict", src_reg, dst_reg
                    )

        # Copy transforms for boolean solids
        for tra_attr in ["tra1", "tra2"]:
            if hasattr(solid, tra_attr) and getattr(solid, tra_attr):
                tra = getattr(solid, tra_attr)
                _copy_named_object(
                    tra.position, "positionDict", "positionDict", src_reg, dst_reg
                )
                _copy_named_object(
                    tra.rotation, "rotationDict", "rotationDict", src_reg, dst_reg
                )

        # Recursively copy constituent solids for boolean operations
        if hasattr(solid, "obj1") and solid.obj1:
            _copy_solid_recursive(solid.obj1, src_reg, dst_reg)
        if hasattr(solid, "obj2") and solid.obj2:
            _copy_solid_recursive(solid.obj2, src_reg, dst_reg)

    _copy_material(lv.material, src_reg, dst_reg)

    if lv.solid:
        _copy_solid_recursive(lv.solid, src_reg, dst_reg)

    # Copy define dictionary entries related to this volume
    if hasattr(src_reg, "defineDict"):
        for key in src_reg.defineDict:
            if lv.name in key and key not in dst_reg.defineDict:
                dst_reg.defineDict[key] = src_reg.defineDict[key]

    # Recursively copy daughter volumes
    if hasattr(lv, "daughterVolumes"):
        for daughter_pv in lv.daughterVolumes:
            if hasattr(daughter_pv, "logicalVolume"):
                daughter_lv = daughter_pv.logicalVolume
                if daughter_lv.name not in dst_reg.logicalVolumeDict:
                    dst_reg.addLogicalVolume(daughter_lv)
                copy_referenced_resources(daughter_lv, src_reg, dst_reg)


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
