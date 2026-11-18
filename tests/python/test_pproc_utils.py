from __future__ import annotations

from lgdo.types import VectorOfVectors
from remage.reshaping import (
    _get_wo_mode,
    _get_wo_mode_forwarded,
    _move_units_to_flattened_data,
)


def test_wo_mode():
    assert _get_wo_mode([], "det1", overwrite=True) == "overwrite_file"
    assert _get_wo_mode([], "det1", overwrite=False) == "write_safe"
    assert _get_wo_mode(["det1"], "det2", overwrite=False) == "append_column"
    assert _get_wo_mode(["det1"], "det1", overwrite=False) == "append"

    assert _get_wo_mode_forwarded([], True, overwrite=True) == "overwrite_file"
    assert _get_wo_mode_forwarded(["a"], True, overwrite=True) == "write_safe"
    assert _get_wo_mode_forwarded([], True, overwrite=False) == "write_safe"
    assert _get_wo_mode_forwarded(["a"], True, overwrite=False) == "write_safe"
    assert _get_wo_mode_forwarded([], False, overwrite=False) == "append"
    assert _get_wo_mode_forwarded(["a"], False, overwrite=False) == "append"


def test_move_units_to_flattened_data():
    data = VectorOfVectors([[1, 2, 3]], attrs={"units": "mm"})
    _move_units_to_flattened_data(data)

    assert data.attrs.get("units") is None
    assert data.flattened_data.attrs.get("units") == "mm"
