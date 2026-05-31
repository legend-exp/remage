# Copyright (C) 2026 The reboost developers
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

import logging
from collections.abc import Sequence

import awkward as ak
import lh5
import numpy as np
from lgdo import Array
from lgdo.types import LGDO, Struct, Table, VectorOfVectors
from reboost.iterator import GLMIterator
from reboost.shape.group import group_by_time

log = logging.getLogger("remage")


def reshape_output(
    stp_files: list[str],
    hit_files: list[str] | str,
    *,
    reshape_tables: Sequence[str],
    forward_tables: Sequence[str],
    out_field: str,
    time_window_in_us: float,
    overwrite: bool = False,
    buffer: int = int(5e6),
) -> None:
    """Post-process remage step files into hit files using reboost.

    For each detector listed in ``reshape_tables``, steps are grouped by event
    id and time (window in microseconds), producing per-hit ``t0`` and ``evtid``
    columns. Tables listed in ``forward_tables`` are copied unchanged.

    Parameters
    ----------
    stp_files
        list of remage step files.
    hit_files
        list of output hit files (one per stp file) or a single file when all
        inputs should be merged into the same output.
    reshape_tables
        detector tables to reshape via time-grouping.
    forward_tables
        auxiliary tables to forward to the output unchanged.
    out_field
        lh5 group under which reshaped detector tables are written.
    time_window_in_us
        coincidence time window in microseconds for hit grouping.
    overwrite
        whether to overwrite the output files.
    buffer
        buffer size (in rows) for the lh5 iterator.
    """
    hit_files_list = (
        [hit_files] * len(stp_files) if isinstance(hit_files, str) else list(hit_files)
    )
    if len(hit_files_list) != len(stp_files):
        msg = "hit_files must be a single path or match stp_files in length"
        raise ValueError(msg)

    for file_idx, (stp_file, hit_file) in enumerate(
        zip(stp_files, hit_files_list, strict=True)
    ):
        msg = f"processing {stp_file} -> {hit_file}"
        log.info(msg)

        new_file = (
            file_idx == 0 or hit_files_list[file_idx] != hit_files_list[file_idx - 1]
        )
        written_tables: set[str] = set()

        for in_det_idx, detector in enumerate(reshape_tables):
            table = f"stp/{detector}"
            if lh5.ls(stp_file, table) == []:
                continue

            iterator = GLMIterator(
                glm_file=None,
                stp_file=stp_file,
                lh5_group=detector,
                start_row=0,
                n_rows=None,
                stp_field="stp",
                buffer=buffer,
                reshaped_files=False,
            )

            for stps, chunk_idx, _ in iterator:
                if stps is None:
                    continue

                hit_table = group_by_time(
                    stps.view_as("ak", with_units=True),
                    window=time_window_in_us,
                )
                # move units from the VoV down to its flattened_data, matching
                # the convention used elsewhere
                for data in hit_table.values():
                    _move_units_to_flattened_data(data)

                hits_ak = hit_table.view_as("ak")
                t0 = ak.fill_none(ak.firsts(hits_ak.time, axis=-1), 0)
                evtid = ak.fill_none(ak.firsts(hits_ak.evtid, axis=-1), 0)
                hit_table.add_field("t0", Array(np.asarray(t0), attrs={"units": "ns"}))
                hit_table.add_field("evtid", Array(np.asarray(evtid)))

                wo_mode = _get_wo_mode(in_det_idx, chunk_idx, new_file, overwrite)
                _write_lh5(hit_table, hit_file, out_field, detector, wo_mode)
                written_tables.add(detector)

        for obj in forward_tables:
            wo_mode = _get_wo_mode_forwarded(written_tables, new_file, overwrite)
            lh5.write(lh5.read(obj, stp_file), obj, hit_file, wo_mode=wo_mode)
            written_tables.add(obj)


def _get_wo_mode(
    in_det_idx: int, chunk_idx: int, new_file: bool, overwrite: bool
) -> str:
    """LH5 write mode for a reshaped detector table chunk.

    For the first chunk of the first detector in a new hit file we (over)write
    the whole file; for the first chunk of a further detector we append a new
    column to the parent struct; otherwise we append rows to the existing
    table.
    """
    if chunk_idx == 0 and new_file:
        if in_det_idx == 0:
            return "overwrite_file" if overwrite else "write_safe"
        return "append_column"
    return "append"


def _get_wo_mode_forwarded(
    written_tables: set[str], new_file: bool, overwrite: bool
) -> str:
    """LH5 write mode for a table that is copied through unchanged."""
    if not new_file:
        return "append"
    if overwrite and not written_tables:
        return "overwrite_file"
    return "write_safe"


def _write_lh5(
    hit_table: Table, file: str, out_field: str, out_detector: str, wo_mode: str
) -> None:
    """Write a hit table at ``out_field/out_detector``.

    On the first write the table is wrapped in a Struct so the parent group is
    created with the right type; subsequent appends target the table directly.
    """
    if wo_mode not in ("a", "append"):
        lh5.write(Struct({out_detector: hit_table}), out_field, file, wo_mode=wo_mode)
    else:
        lh5.write(hit_table, f"{out_field}/{out_detector}", file, wo_mode=wo_mode)


def _move_units_to_flattened_data(data: LGDO) -> None:
    """Recursively move a ``units`` attr from a VectorOfVectors to its flattened_data."""
    if isinstance(data, VectorOfVectors) and "units" in data.attrs:
        unit = data.attrs.pop("units")
        data.flattened_data.attrs["units"] = unit
        if isinstance(data.flattened_data, VectorOfVectors):
            _move_units_to_flattened_data(data.flattened_data)
