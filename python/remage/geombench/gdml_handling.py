from __future__ import annotations

import tempfile
from pathlib import Path

import pyg4ometry

## GDML Handling Functions - Helper utilities for copying registry resources

def _ensure_dict_exists(registry: pyg4ometry.geant4.Registry, dict_name: str) -> dict:
    """Ensure a dictionary attribute exists on a registry object."""
    if not hasattr(registry, dict_name):
        setattr(registry, dict_name, {})
    return getattr(registry, dict_name)


def _copy_to_dict_if_missing(src_dict: dict, dst_dict: dict, key: str) -> None:
    """Copy a key from source dict to destination dict if not already present."""
    if key in src_dict and key not in dst_dict:
        dst_dict[key] = src_dict[key]


def _copy_position(position: pyg4ometry.geant4.Position, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Copy a position definition from source to destination registry."""
    if not (position and hasattr(position, "name")):
        return

    if position.name in src_reg.positionDict:
        dst_pos_dict = _ensure_dict_exists(dst_reg, "positionDict")
        _copy_to_dict_if_missing(src_reg.positionDict, dst_pos_dict, position.name)


def _copy_rotation(rotation: pyg4ometry.geant4.Rotation, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Copy a rotation definition from source to destination registry."""
    if not (rotation and hasattr(rotation, "name")):
        return

    if rotation.name in src_reg.rotationDict:
        dst_rot_dict = _ensure_dict_exists(dst_reg, "rotationDict")
        _copy_to_dict_if_missing(src_reg.rotationDict, dst_rot_dict, rotation.name)


def _copy_material_properties(material: pyg4ometry.geant4.Material, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Copy material property definitions (e.g., RINDEX matrices)."""
    if not (hasattr(material, "properties") and material.properties):
        return

    for prop_value in material.properties.values():
        if not hasattr(prop_value, "name"):
            continue

        for dict_name in ["matrixDict", "defineDict"]:
            src_dict = getattr(src_reg, dict_name, {})
            if prop_value.name in src_dict:
                dst_dict = _ensure_dict_exists(dst_reg, dict_name)
                _copy_to_dict_if_missing(src_dict, dst_dict, prop_value.name)


def _copy_material(material: pyg4ometry.geant4.Material, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Copy a material and its dependencies to the destination registry."""
    if not material or material.name in dst_reg.materialDict:
        return

    _copy_material_properties(material, src_reg, dst_reg)
    material.registry = dst_reg
    dst_reg.addMaterial(material)


def _copy_solid_positions(solid: pyg4ometry.geant4.Solid, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Copy position lists referenced by certain solid types."""
    # For GenericPolycone, Tessellated, etc that reference position lists
    if hasattr(solid, "pPositions") and solid.pPositions:
        for pos in solid.pPositions:
            _copy_position(pos, src_reg, dst_reg)

    # For Tessellated solids with vertex positions
    if hasattr(solid, "vertices") and solid.vertices:
        for vertex in solid.vertices:
            _copy_position(vertex, src_reg, dst_reg)


def _copy_boolean_transforms(solid: pyg4ometry.geant4.Solid, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Copy position and rotation transforms for boolean solid operations."""
    for tra_attr in ["tra1", "tra2"]:
        if not hasattr(solid, tra_attr):
            continue

        tra = getattr(solid, tra_attr)
        if not tra:
            continue

        if hasattr(tra, "position"):
            _copy_position(tra.position, src_reg, dst_reg)

        if hasattr(tra, "rotation"):
            _copy_rotation(tra.rotation, src_reg, dst_reg)


def _copy_solid_recursive(solid: pyg4ometry.geant4.Solid, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Recursively copy a solid and all its nested components."""
    if solid.name in dst_reg.solidDict:
        return

    solid.registry = dst_reg
    dst_reg.addSolid(solid)

    _copy_solid_positions(solid, src_reg, dst_reg)

    # For boolean solids, recursively copy constituent solids
    if hasattr(solid, "obj1") and solid.obj1:
        _copy_solid_recursive(solid.obj1, src_reg, dst_reg)
    if hasattr(solid, "obj2") and solid.obj2:
        _copy_solid_recursive(solid.obj2, src_reg, dst_reg)

    _copy_boolean_transforms(solid, src_reg, dst_reg)


def _copy_defines_for_volume(lv: pyg4ometry.geant4.LogicalVolume, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Copy define dictionary entries related to a logical volume."""
    if not hasattr(src_reg, "defineDict"):
        return

    for key in src_reg.defineDict.keys():
        if lv.name in key:
            dst_reg.defineDict[key] = src_reg.defineDict[key]


def _add_logical_volume_to_registry(lv: pyg4ometry.geant4.LogicalVolume, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Add a logical volume to the registry if not already present."""
    if lv.name in dst_reg.logicalVolumeDict:
        return

    dst_reg.addLogicalVolume(lv)
    if lv.name not in dst_reg.logicalVolumeList:
        dst_reg.logicalVolumeList.append(lv.name)
    dst_reg.logicalVolumeDict[lv.name] = lv


def _copy_daughter_volumes(lv: pyg4ometry.geant4.LogicalVolume, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
    """Recursively copy all daughter volumes and their resources."""
    if not hasattr(lv, "daughterVolumes"):
        return

    for daughter_pv in lv.daughterVolumes:
        if not hasattr(daughter_pv, "logicalVolume"):
            continue

        daughter_lv = daughter_pv.logicalVolume
        _add_logical_volume_to_registry(daughter_lv, dst_reg)
        copy_referenced_resources(daughter_lv, src_reg, dst_reg)


def copy_referenced_resources(lv: pyg4ometry.geant4.LogicalVolume, src_reg: pyg4ometry.geant4.Registry, dst_reg: pyg4ometry.geant4.Registry) -> None:
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
    _copy_material(lv.material, src_reg, dst_reg)
    _copy_defines_for_volume(lv, src_reg, dst_reg)

    if lv.solid:
        _copy_solid_recursive(lv.solid, src_reg, dst_reg)

    _copy_daughter_volumes(lv, src_reg, dst_reg)


## GDML Handling Functions - High-level operations


def load_gdml_geometry(gdml_path: Path, object_name: str = "object_lv") -> dict:
    """Load a GDML geometry file and rename the world volume.

    Parameters
    ----------
    gdml_path :
        Path to the GDML file.
    object_name : optional
        Name to assign to the loaded world volume. Default is "object_lv".

    Returns
    -------
    dict
        Dictionary with keys:
        - "object_lv": The loaded and renamed logical volume
        - "registry": The pyg4ometry registry containing the geometry
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

    return {"object_lv": world_volume, "registry": registry}


def _create_world_box(
    extent, buffer_fraction: float, registry
) -> pyg4ometry.geant4.LogicalVolume:
    """Create a world box volume with buffered dimensions."""
    width = (extent[1][0] - extent[0][0]) * (1 + buffer_fraction)
    height = (extent[1][1] - extent[0][1]) * (1 + buffer_fraction)
    depth = (extent[1][2] - extent[0][2]) * (1 + buffer_fraction)

    box_solid = pyg4ometry.geant4.solid.Box(
        "world_box", width, height, depth, registry, "mm"
    )
    world_lv = pyg4ometry.geant4.LogicalVolume(
        box_solid, "G4_Galactic", "world_lv", registry
    )
    return world_lv


def _calculate_centering_offset(extent) -> list[float]:
    """Calculate the offset needed to center an object at world origin."""
    return [
        -(extent[0][0] + extent[1][0]) / 2.0,
        -(extent[0][1] + extent[1][1]) / 2.0,
        -(extent[0][2] + extent[1][2]) / 2.0,
    ]


def _ensure_volume_in_registry(volume: pyg4ometry.geant4.LogicalVolume, registry: pyg4ometry.geant4.Registry) -> None:
    """Ensure a volume and its hierarchy are properly registered."""
    registry.addLogicalVolume(volume)
    registry.addVolumeRecursive(volume)

    # Ensure all volumes are in the list
    for vol_name in registry.logicalVolumeDict.keys():
        if vol_name not in registry.logicalVolumeList:
            registry.logicalVolumeList.append(vol_name)


def position_object_in_world(
    geometry: dict, buffer_fraction: float = 0, object_name: str = "object_lv"
) -> dict:
    """Position an object in a new world volume with optional buffer space.

    Creates a new world volume sized to fit the object with optional buffer,
    and places the object centered within it.

    Parameters
    ----------
    geometry: 
        Dictionary with "object_lv" and "registry" keys containing the geometry.
    buffer_fraction : optional
        Fractional buffer to add around the object (0 = no buffer, 0.25 = 25% extra space).
    object_name : optional
        Name for the object logical volume. Default is "object_lv".

    Returns
    -------
    dict
        Dictionary with keys:
        - "object_lv": The new world logical volume
        - "registry": New registry containing the positioned geometry
    """
    object_lv = geometry["object_lv"]
    object_lv.name = object_name

    extent = object_lv.extent(True)
    object_pv_name = object_name.rstrip("_lv") + "_pv"

    # Create new registry with world volume
    new_registry = pyg4ometry.geant4.Registry()
    world_lv = _create_world_box(extent, buffer_fraction, new_registry)
    new_registry.setWorld(world_lv.name)

    # Add object volume and its hierarchy
    _ensure_volume_in_registry(object_lv, new_registry)

    # Place object centered in world
    centering_offset = _calculate_centering_offset(extent)
    pyg4ometry.geant4.PhysicalVolume(
        [0, 0, 0], centering_offset, object_lv, object_pv_name, world_lv, new_registry
    )

    # Copy all referenced resources
    copy_referenced_resources(object_lv, geometry["registry"], new_registry)

    return {"object_lv": world_lv, "registry": new_registry}


def extract_component_from_gdml(geometry: dict, lv_name: str) -> dict:
    """Extract a specific logical volume component from a GDML geometry.

    Returns the extracted component without wrapping in a world volume.
    Use generate_tmp_gdml_geometry to position it in a buffered world.

    Parameters
    ----------
    geometry :
        Dictionary containing the loaded geometry with keys "object_lv" and "registry".
    lv_name : 
        Name of the logical volume to extract.

    Returns
    -------
    dict
        Dictionary with keys:
        - "object_lv": The extracted logical volume
        - "registry": The source registry containing the component

    Raises
    ------
    ValueError
        If the specified logical volume name is not found in the registry.
    """
    registry = geometry["registry"]

    if lv_name not in registry.logicalVolumeDict:
        msg = f"Logical volume '{lv_name}' not found in the geometry registry."
        raise ValueError(msg)

    return {"object_lv": registry.logicalVolumeDict[lv_name], "registry": registry}


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
    positioned_geometry = position_object_in_world(
        geometry, buffer_fraction=buffer_fraction, object_name=object_name
    )

    writer = pyg4ometry.gdml.Writer()
    writer.addDetector(positioned_geometry["registry"])

    temp_file = tempfile.NamedTemporaryFile(mode="w", delete=False, suffix=".gdml")
    tempfile_path = Path(temp_file.name)
    writer.write(tempfile_path)

    return tempfile_path
