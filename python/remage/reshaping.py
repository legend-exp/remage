# Copyright (C) 2026 The reboost developers
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <https://www.gnu.org/licenses/>.

from __future__ import annotations

import itertools
import logging
from collections.abc import Sequence

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
        lh5 group holding the detector tables, both in the input and in the
        output files.
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
        maximum number of steps read from a step file at once. It is also the
        maximum number of steps whose hits are collected in memory before they
        are written, so it sets the memory use.
    """
    hit_files_list = (
        [hit_files] * len(stp_files) if isinstance(hit_files, str) else list(hit_files)
    )
    if len(hit_files_list) != len(stp_files):
        msg = "hit_files must be a single path or match stp_files in length"
        raise ValueError(msg)

    # step files that end up in the same hit file
    stp_files_of: dict[str, list[str]] = {}
    for stp_file, hit_file in zip(stp_files, hit_files_list, strict=True):
        stp_files_of.setdefault(str(hit_file), []).append(stp_file)

    for hit_file, inputs in stp_files_of.items():
        msg = f"processing {', '.join(inputs)} -> {hit_file}"
        log.info(msg)

        # tables already written to this hit file, in order: the first write
        # creates the file, the first write of a further table appends a column
        written_tables: list[str] = []

        for detector in reshape_tables:
            table = f"{out_field}/{detector}"
            # writing a table has a large fixed cost, so the hits from all step
            # files are collected and written together, at most ``buffer`` steps
            # at a time
            pending: list[Table] = []
            n_pending_steps = 0

            chunks = (
                stps
                for stp_file in inputs
                if lh5.ls(stp_file, table) != []
                for stps in _iter_event_aligned_chunks(stp_file, table, buffer)
            )
            # write what is pending before a chunk that would exceed ``buffer``
            # steps; the trailing None triggers the write of what is left
            for stps in itertools.chain(chunks, [None]):
                if pending and (stps is None or n_pending_steps + len(stps) > buffer):
                    hit_table = _concatenate_hit_tables(pending)
                    pending, n_pending_steps = [], 0

                    # move units from the VoV down to its flattened_data, matching
                    # the convention used elsewhere
                    for data in hit_table.values():
                        _move_units_to_flattened_data(data)

                    # time and evtid of the first step of each hit
                    cumlen = hit_table["time"].cumulative_length.nda
                    first = cumlen - np.diff(cumlen, prepend=0)
                    t0 = hit_table["time"].flattened_data.nda[first]
                    evtid = hit_table["evtid"].flattened_data.nda[first]
                    hit_table.add_field("t0", Array(t0, attrs={"units": "ns"}))
                    hit_table.add_field("evtid", Array(evtid))

                    wo_mode = _get_wo_mode(written_tables, detector, overwrite)
                    _write_lh5(hit_table, hit_file, out_field, detector, wo_mode)
                    if detector not in written_tables:
                        written_tables.append(detector)

                if stps is not None:
                    pending.append(_group_by_time(stps, window_us=time_window_in_us))
                    n_pending_steps += len(stps)

        for detector in flat_hit_tables:
            table = f"{out_field}/{detector}"
            for stp_file in inputs:
                if lh5.ls(stp_file, table) == []:
                    continue

                for stps in _iter_chunks(stp_file, table, buffer):
                    # already one hit per detector per event: forward as-is, only
                    # renaming the time column so it can join the coincidence map
                    hit_table = _calorimeter_hit_table(stps)
                    wo_mode = _get_wo_mode(written_tables, detector, overwrite)
                    _write_lh5(hit_table, hit_file, out_field, detector, wo_mode)
                    if detector not in written_tables:
                        written_tables.append(detector)

        for i, stp_file in enumerate(inputs):
            for obj in forward_tables:
                wo_mode = _get_wo_mode_forwarded(written_tables, i == 0, overwrite)
                lh5.write(lh5.read(obj, stp_file), obj, hit_file, wo_mode=wo_mode)
                written_tables.append(obj)


def _get_wo_mode(written_tables: list[str], detector: str, overwrite: bool) -> str:
    """LH5 write mode for a detector table, given the tables already in the hit file.

    The first write to the hit file (over)writes the whole file; the first
    write of a further detector appends a new column to the parent struct;
    otherwise we append rows to the existing table.
    """
    if detector in written_tables:
        return "append"
    if not written_tables:
        return "overwrite_file" if overwrite else "write_safe"
    return "append_column"


def _get_wo_mode_forwarded(
    written_tables: list[str], new_file: bool, overwrite: bool
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


def _iter_chunks(stp_file: str, table: str, buffer: int):
    """Yield ``buffer``-sized row chunks of ``table``.

    Used for flat hit tables (e.g. calorimeter) where every row is an
    independent hit, so no evtid alignment is needed and the table can be
    streamed in fixed-size blocks to bound memory.
    """
    # read_n_rows returns None for a Table group, so count rows on a column
    columns = lh5.ls(stp_file, f"{table}/*")
    if not columns:
        return
    n_total = lh5.read_n_rows(columns[0], stp_file) or 0
    start = 0
    while start < n_total:
        n_rows = min(buffer, n_total - start)
        yield lh5.read(table, stp_file, start_row=start, n_rows=n_rows)
        start += n_rows


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

    # a table that fits in one chunk is read only once
    stps = lh5.read(table, stp_file) if n_total <= buffer else None
    evtids = stps["evtid"] if stps is not None else lh5.read(f"{table}/evtid", stp_file)
    diffs = np.diff(evtids.nda)
    if np.any(diffs < 0):
        msg = (
            f"evtids in {table} of {stp_file} are not monotonically increasing: "
            "an event's steps would span separate blocks, breaking per-chunk "
            "hit grouping"
        )
        raise ValueError(msg)
    if stps is not None:
        yield stps
        return

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


def _group_by_time(stps: Table, *, window_us: float) -> Table:
    """Group steps into hits by ``evtid`` and a coincidence time window.

    A new hit starts whenever ``evtid`` changes, or ``time`` jumps by more than
    ``window_us`` microseconds within the same evtid. Returns an :class:`lgdo.Table`
    of :class:`VectorOfVectors`, one per column of ``stps``, preserving each
    column's ``units`` attribute.

    ``time`` is assumed to be in nanoseconds (the unit remage writes). Only flat
    columns (one-dimensional :class:`lgdo.Array`) are supported.
    """
    fields = list(stps)
    for f in fields:
        col = stps[f]
        if not isinstance(col, Array) or col.nda.ndim != 1:
            msg = (
                f"cannot group step column {f!r} of type {type(col).__name__}: "
                "only flat columns (one-dimensional lgdo.Array) are supported"
            )
            raise TypeError(msg)

    units_dict = {f: stps[f].attrs.get("units") for f in fields}
    if units_dict.get("time") != "ns":
        msg = f"expected time field in 'ns', got {units_dict.get('time')!r}"
        raise ValueError(msg)

    # sort first by evtid, then by time within each evtid
    order = np.lexsort((stps["time"].nda, stps["evtid"].nda))
    sorted_cols = {f: stps[f].nda[order] for f in fields}

    window_ns = window_us * 1000.0
    time = sorted_cols["time"]
    evtid = sorted_cols["evtid"]
    dt = np.diff(time)
    de = np.diff(evtid)
    boundary = ((dt > window_ns) & (de == 0)) | (de > 0)
    # an empty step table has no hits, not one empty hit
    if len(time) == 0:
        cumulative_length = np.empty(0, dtype=np.int64)
    else:
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


def _concatenate_hit_tables(tables: list[Table]) -> Table:
    """Concatenate the rows of hit tables returned by :func:`_group_by_time`."""
    if len(tables) == 1:
        return tables[0]

    if any(set(t) != set(tables[0]) for t in tables):
        msg = "the step tables to merge do not have the same columns"
        raise ValueError(msg)

    out = Table(size=sum(len(t) for t in tables))
    for field, first in tables[0].items():
        cols = [t[field] for t in tables]
        # each cumulative_length restarts from zero: shift it by the number of
        # steps in the tables before
        n_steps = np.cumsum([0] + [len(c.flattened_data) for c in cols[:-1]])
        out.add_field(
            field,
            VectorOfVectors(
                cumulative_length=np.concatenate(
                    [
                        c.cumulative_length.nda + n
                        for c, n in zip(cols, n_steps, strict=True)
                    ]
                ),
                flattened_data=np.concatenate([c.flattened_data.nda for c in cols]),
                attrs=first.attrs,
            ),
        )
    return out


def _move_units_to_flattened_data(data: LGDO) -> None:
    """Recursively move a ``units`` attr from a VectorOfVectors to its flattened_data."""
    if isinstance(data, VectorOfVectors) and "units" in data.attrs:
        unit = data.attrs.pop("units")
        data.flattened_data.attrs["units"] = unit
        if isinstance(data.flattened_data, VectorOfVectors):
            _move_units_to_flattened_data(data.flattened_data)
