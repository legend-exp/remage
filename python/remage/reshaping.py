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

log = logging.getLogger("remage")


def reshape_output(
    stp_files: list[str],
    hit_files: list[str] | str,
    *,
    reshape_tables: Sequence[str],
    forward_tables: Sequence[str],
    out_field: str,
    time_window_in_us: float,
    flat_hit_tables: Sequence[str] = (),
    overwrite: bool = False,
    buffer: int = int(5e6),
) -> None:
    """Post-process remage step files into reshaped hit files.

    For each detector listed in ``reshape_tables``, steps are grouped by event
    id and time (window in microseconds), producing per-hit ``t0`` and ``evtid``
    columns. Detectors in ``flat_hit_tables`` already hold one hit per detector
    per event and are written alongside the reshaped detectors with their
    ``time`` column renamed to ``t0`` (no step-grouping). Tables listed in
    ``forward_tables`` are copied unchanged.

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
    flat_hit_tables
        detector tables that already contain one hit per detector per event
        (e.g. calorimeter output) and must not be step-grouped. They are written
        next to the reshaped detectors under ``out_field`` with ``time`` renamed
        to ``t0`` so they can join the time-coincidence map.
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
        # number of detector tables already written to this hit file; drives the
        # "first write creates the file, the rest append a column" logic
        n_det_written = 0

        for detector in reshape_tables:
            table = f"stp/{detector}"
            if lh5.ls(stp_file, table) == []:
                continue

            wrote_any = False
            for chunk_idx, stps in enumerate(
                _iter_event_aligned_chunks(stp_file, table, buffer)
            ):
                hit_table = _group_by_time(
                    stps.view_as("ak", with_units=True),
                    window_us=time_window_in_us,
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

                wo_mode = _get_wo_mode(
                    n_det_written == 0, chunk_idx, new_file, overwrite
                )
                _write_lh5(hit_table, hit_file, out_field, detector, wo_mode)
                wrote_any = True

            if wrote_any:
                written_tables.add(detector)
                n_det_written += 1

        for detector in flat_hit_tables:
            table = f"stp/{detector}"
            if lh5.ls(stp_file, table) == []:
                continue

            # already one hit per detector per event: forward as-is, only
            # renaming the time column so it can join the coincidence map
            hit_table = _calorimeter_hit_table(lh5.read(table, stp_file))
            wo_mode = _get_wo_mode(n_det_written == 0, 0, new_file, overwrite)
            _write_lh5(hit_table, hit_file, out_field, detector, wo_mode)
            written_tables.add(detector)
            n_det_written += 1

        for obj in forward_tables:
            wo_mode = _get_wo_mode_forwarded(written_tables, new_file, overwrite)
            lh5.write(lh5.read(obj, stp_file), obj, hit_file, wo_mode=wo_mode)
            written_tables.add(obj)


def _get_wo_mode(
    first_write: bool, chunk_idx: int, new_file: bool, overwrite: bool
) -> str:
    """LH5 write mode for a detector table chunk.

    For the first chunk of the first detector in a new hit file we (over)write
    the whole file; for the first chunk of a further detector we append a new
    column to the parent struct; otherwise we append rows to the existing
    table.
    """
    if chunk_idx == 0 and new_file:
        if first_write:
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


def _calorimeter_hit_table(stps: Table) -> Table:
    """Build a flat hit table from an already per-hit detector table.

    Calorimeter-like output accumulates exactly one hit per event. It must not be
    step-grouped; we only rename the ``time`` column to ``t0`` for the TCM.
    """
    out = Table(size=stps.size)
    for field in stps:
        out.add_field("t0" if field == "time" else field, stps[field])
    return out


def _iter_event_aligned_chunks(stp_file: str, table: str, buffer: int):
    """Yield ``buffer``-sized chunks of ``table`` aligned to evtid boundaries.

    All steps of a given evtid stay in the same chunk, so each chunk can be
    grouped into hits independently. The full evtid column is read once up front
    to compute the split points.

    This relies on remage writing evtids monotonically increasing.
    """
    n_total = lh5.read_n_rows(f"{table}/evtid", stp_file) or 0
    if n_total == 0:
        return

    evtids = lh5.read(f"{table}/evtid", stp_file).nda
    diffs = np.diff(evtids)
    if np.any(diffs < 0):
        msg = (
            f"evtids in {table} of {stp_file} are not monotonically increasing: "
            "an event's steps would span separate blocks, breaking per-chunk "
            "hit grouping"
        )
        raise ValueError(msg)
    event_starts = np.concatenate(([0], np.flatnonzero(diffs) + 1, [n_total]))

    start = 0
    while start < n_total:
        target = start + buffer
        pos = int(np.searchsorted(event_starts, target, side="right")) - 1
        end = int(event_starts[pos])
        if end <= start:
            # buffer smaller than a single event - take the whole event
            end = int(event_starts[pos + 1])
        yield lh5.read(table, stp_file, start_row=start, n_rows=end - start)
        start = end


def _group_by_time(obj: ak.Array, *, window_us: float) -> Table:
    """Group steps into hits by ``evtid`` and a coincidence time window.

    A new hit starts whenever ``evtid`` changes, or ``time`` jumps by more than
    ``window_us`` microseconds within the same evtid. Returns an :class:`lgdo.Table`
    of :class:`VectorOfVectors`, one per field of ``obj``, preserving each
    field's ``units`` parameter.

    ``time`` is assumed to be in nanoseconds (the unit remage writes).
    """
    fields = obj.fields
    units_dict = {f: ak.parameters(obj[f]).get("units") for f in fields}
    if units_dict.get("time") != "ns":
        msg = f"expected time field in 'ns', got {units_dict.get('time')!r}"
        raise ValueError(msg)

    # sort first by evtid, then by time within each evtid
    by_evtid = obj[ak.argsort(obj["evtid"])]
    grouped = ak.unflatten(by_evtid, ak.run_lengths(by_evtid["evtid"]))
    time_order = ak.argsort(grouped["time"], axis=-1)
    sorted_cols = {f: np.asarray(ak.flatten(grouped[f][time_order])) for f in fields}

    window_ns = window_us * 1000.0
    time = sorted_cols["time"]
    evtid = sorted_cols["evtid"]
    dt = np.diff(time)
    de = np.diff(evtid)
    boundary = ((dt > window_ns) & (de == 0)) | (de > 0)
    cumulative_length = np.append(np.where(boundary)[0] + 1, len(time))

    out = Table(size=len(cumulative_length))
    for f in fields:
        vov = VectorOfVectors(
            cumulative_length=cumulative_length, flattened_data=sorted_cols[f]
        )
        if units_dict[f] is not None:
            vov.attrs["units"] = units_dict[f]
        out.add_field(f, vov)
    return out


def _move_units_to_flattened_data(data: LGDO) -> None:
    """Recursively move a ``units`` attr from a VectorOfVectors to its flattened_data."""
    if isinstance(data, VectorOfVectors) and "units" in data.attrs:
        unit = data.attrs.pop("units")
        data.flattened_data.attrs["units"] = unit
        if isinstance(data.flattened_data, VectorOfVectors):
            _move_units_to_flattened_data(data.flattened_data)
