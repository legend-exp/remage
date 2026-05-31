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
from reboost.iterator import GLMIterator
from reboost.shape.group import group_by_time
from reboost.units import move_units_to_flattened_data
from reboost.utils import (
    get_wo_mode,
    get_wo_mode_forwarded,
    is_new_hit_file,
    write_lh5,
)

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

    files_like = type("Files", (), {"hit": hit_files_list})()

    for file_idx, (stp_file, hit_file) in enumerate(
        zip(stp_files, hit_files_list, strict=True)
    ):
        msg = f"processing {stp_file} -> {hit_file}"
        log.info(msg)

        new_file = is_new_hit_file(files_like, file_idx)
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
                # move units from the VoV to its flattened_data, matching the
                # convention used elsewhere by reboost
                for data in hit_table.values():
                    move_units_to_flattened_data(data)

                hits_ak = hit_table.view_as("ak")
                t0 = ak.fill_none(ak.firsts(hits_ak.time, axis=-1), 0)
                evtid = ak.fill_none(ak.firsts(hits_ak.evtid, axis=-1), 0)
                hit_table.add_field("t0", Array(np.asarray(t0), attrs={"units": "ns"}))
                hit_table.add_field("evtid", Array(np.asarray(evtid)))

                wo_mode = get_wo_mode(
                    group=0,
                    out_det=0,
                    in_det=in_det_idx,
                    chunk=chunk_idx,
                    new_hit_file=new_file,
                    overwrite=overwrite,
                )
                write_lh5(
                    hit_table,
                    hit_file,
                    time_dict=None,
                    out_field=out_field,
                    out_detector=detector,
                    wo_mode=wo_mode,
                )
                written_tables.add(detector)

        for obj in forward_tables:
            wo_mode = get_wo_mode_forwarded(written_tables, new_file, overwrite)
            lh5.write(lh5.read(obj, stp_file), obj, hit_file, wo_mode=wo_mode)
            written_tables.add(obj)
