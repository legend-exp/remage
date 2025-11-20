from __future__ import annotations

from lgdo import lh5

file_vols = "test-native-volume.mac.lh5"
file_mass = "test-native-volume-mass.mac.lh5"


def get_ratio(file):
    box = lh5.read_as("/stp/Box", file, "ak")
    orb = lh5.read_as("/stp/Orb", file, "ak")
    return len(box) / len(orb)


raise ValueError(get_ratio(file_vols), get_ratio(file_mass))
