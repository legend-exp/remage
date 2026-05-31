# Copyright (C) 2024 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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
import time
from collections.abc import Sequence
from contextlib import contextmanager
from pathlib import Path

import awkward as ak
import h5py
import lh5
import numpy as np
import pygama.evt
from lgdo import Array, Scalar, Struct
from lh5.io.concat import lh5concat
from reboost.iterator import GLMIterator
from reboost.shape.group import group_by_time
from reboost.units import move_units_to_flattened_data
from reboost.utils import (
    get_wo_mode,
    get_wo_mode_forwarded,
    is_new_hit_file,
    write_lh5,
)

from . import utils
from .ipc import IpcResult

log = logging.getLogger("remage")


def post_proc(
    ipc_info: IpcResult,
    flat_output: bool,
    merge_output_files: bool,
    time_window_in_us: float,
) -> None:
    remage_files: list[str] = ipc_info.get("output")
    main_output_file: str | None = ipc_info.get_single("output_main")
    overwrite_output: bool = ipc_info.get_single("overwrite_output", "0") == "1"
    det_tables_path: str | None = ipc_info.get_single("ntuple_output_directory")

    ipc_info.remove("output_main")

    if main_output_file is None:
        return

    assert det_tables_path is not None

    output_file_exts = {
        Path(p).suffix.lower() for p in [*remage_files, main_output_file]
    }

    detector_info: list[str] = ipc_info.get("output_ntuple", 2)
    detector_info_aux: list[str] = ipc_info.get("output_ntuple_aux", 2)

    assert len(output_file_exts) == 1

    if output_file_exts != {".lh5"}:
        if not flat_output or merge_output_files:
            log.error(
                "Merging or reshaping is not supported for output format %s",
                next(iter(output_file_exts)).lstrip("."),
            )

        return

    # LH5 output post-processing
    assert (len(remage_files) == 0 and main_output_file is None) or (
        len(remage_files) > 0 and main_output_file is not None
    )

    if flat_output and not merge_output_files:
        return

    # RMGConvertLH5 informs up about where the soft links to output tables are stored
    # we are going to use them to build the TCM
    lh5_links_group_name: str = (
        det_tables_path + "/" + ipc_info.get("lh5_links_group_name")[0]
    )
    lh5_event_number_name: str = "/" + ipc_info.get("lh5_event_number_name")[0]

    time_start = time.time()

    # if merging is on, write everything to a single file
    output_files: list[str] | str = (
        remage_files if not merge_output_files else main_output_file
    )

    if not flat_output:
        msg = (
            "Reshaping "
            + ("and merging " if merge_output_files else "")
            + "output files"
        )
        log.info(msg)

        # registered scintillator or germanium detectors
        registered_detectors = list({det[1] for det in detector_info})

        # extract the additional tables in the output file (not detectors)
        extra_tables = list({det[1] for det in detector_info_aux})

        with tmp_renamed_files(remage_files) as original_files:
            # post-process outputs: reshape detector tables by time-grouping
            # and forward auxiliary tables unchanged
            build_hit(
                stp_files=original_files,
                hit_files=output_files,
                reshape_tables=registered_detectors,
                forward_tables=extra_tables,
                out_field=det_tables_path,
                time_window_in_us=time_window_in_us,
                overwrite=overwrite_output,
            )

            # also copy __by_uid__ group to the output files
            # we do this here and not in reboost, as the links require special syntax
            # NOTE: using just the first original file since the links are always the same
            copy_links(original_files[0], output_files, lh5_links_group_name)
            update_number_of_simulated_events(
                original_files, output_files, lh5_event_number_name
            )

        # add a time-coincidence map to the output file(s)
        msg = "Computing and storing the TCM as /tcm"
        log.info(msg)

        for file in utils._to_list(output_files):
            # do not compute the TCM if there are no stepping tables
            if lh5.ls(file, rf"{lh5_links_group_name}/det*") != []:
                # use tables keyed by UID in the __by_uid__ group.  in this way, the
                # TCM will index tables by UID.  the coincidence criterium is based
                # on Geant4 event identifier and time of the hits
                # NOTE: uses the same time window as in build_hit() reshaping
                pygama.evt.build_tcm(
                    [(file, rf"{lh5_links_group_name}/*")],  # input_tables
                    ["evtid", "t0"],  # coin_cols
                    hash_func=rf"(?<={lh5_links_group_name}/det)\d+",
                    coin_windows=[0, time_window_in_us * 1000],
                    out_file=file,
                    wo_mode="write_safe",
                )

        # set the output file(s) for downstream consumers.
        ipc_info.set("output", output_files)

    if flat_output and merge_output_files:
        msg = "Merging output files"
        log.info(msg)

        with tmp_renamed_files(remage_files) as original_files:
            lh5concat(
                lh5_files=original_files,
                output=main_output_file,
                overwrite=overwrite_output,
                exclude_list=[
                    f"{lh5_links_group_name}/*",
                    f"{lh5_event_number_name}/*",
                ],
            )
            # also copy __by_uid__ group to the output files
            # we do this here and not in reboost, as lh5concat does not copy links correctly.
            # NOTE: using just the first original file since the links are always the same
            copy_links(original_files[0], main_output_file, lh5_links_group_name)
            update_number_of_simulated_events(
                original_files, output_files, lh5_event_number_name
            )

        ipc_info.set("output", main_output_file)

    # deduplicate entries of the process table.
    ntuples_to_deduplicate = set(ipc_info.get("output_ntuple_deduplicate"))
    for file in utils._to_list(output_files):
        for table in ntuples_to_deduplicate:
            deduplicate_table(file, table, "name", not flat_output)

    msg = f"Finished post-processing which took {int(time.time() - time_start)} s"
    log.info(msg)


def copy_links(
    original_file: str, output_files: str | list[str], lh5_links_group_name: str
) -> None:
    if lh5.ls(original_file, lh5_links_group_name) == []:
        return

    with h5py.File(original_file, "r") as inf:
        for file in utils._to_list(output_files):
            with h5py.File(file, "a") as ouf:
                inf.copy(
                    lh5_links_group_name,
                    ouf,
                    lh5_links_group_name,
                    expand_soft=False,  # do _not_ follow soft-links; preserve them
                    expand_external=False,  # likewise for external links
                    expand_refs=False,  # likewise for object-reference datasets
                )

                # remove broken symlinks
                links_group = ouf[lh5_links_group_name]
                for link_name in links_group:
                    link = links_group.get(link_name, getlink=True)
                    if isinstance(link, h5py.SoftLink) and link.path not in ouf:
                        msg = f"removing broken symlink {link_name} -> {link.path}"
                        log.debug(msg)
                        del links_group[link_name]


def update_number_of_simulated_events(
    original_files: list[str], output_files: str | list[str], lh5_event_number_name: str
) -> None:
    output_files = utils._to_list(output_files)
    original_files = utils._to_list(original_files)

    # we can either handle the case of one single output file, or the same number of input/output files.
    if len(output_files) != len(original_files):
        assert len(output_files) == 1
        output_files = output_files * len(original_files)
    assert len(output_files) == len(original_files)

    if lh5.ls(original_files[0], lh5_event_number_name) == []:
        return

    # add up the number of events from the input files.
    output_n_ev = {}
    for f_in, f_out in zip(original_files, output_files, strict=True):
        n_ev = lh5.read(lh5_event_number_name, f_in).value
        assert isinstance(n_ev, np.int64)

        if f_out not in output_n_ev:
            output_n_ev[f_out] = 0
        output_n_ev[f_out] += n_ev

    for file, total_n_ev in output_n_ev.items():
        lh5.write(Scalar(total_n_ev), lh5_event_number_name, file, wo_mode="overwrite")


def deduplicate_table(
    file: str, table_name: str, unique_col: str, to_struct: bool
) -> None:
    table = lh5.read(table_name, file)
    table_old = {
        col: (table[col].view_as("np").copy(), table[col].attrs) for col in table
    }
    _, uniq_idx = np.unique(table_old[unique_col][0], return_index=True)
    table.resize(len(uniq_idx))
    for col, (nda, attrs) in table_old.items():
        table[col] = Array(nda[uniq_idx], attrs=attrs)

    if to_struct:
        keys = list(set(table.keys()) - {unique_col})
        d = {}
        for idx in range(table.size):
            scalars = {}
            for col in keys:
                attrs = {}
                if "units" in table[col].attrs:
                    attrs = {"units": table[col].attrs["units"]}
                scalars[col] = Scalar(table[col].nda[idx], attrs=attrs)

            obj = Struct(scalars) if len(keys) > 1 else scalars[keys[0]]
            d[table[unique_col].nda[idx].decode("utf-8")] = obj

        lh5.write(Struct(d), table_name, file, wo_mode="overwrite")
    else:
        lh5.write(table, table_name, file, wo_mode="overwrite")


def build_hit(
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


def make_tmp(files: list[str] | str) -> list[str]:
    """Hide files.

    Prepend a `.` to their name and rename them on disk.
    """
    files = utils._to_list(files)
    renamed_files = []

    for f in files:
        path = Path(f)
        new_path = path.with_name("." + path.name)
        path.rename(new_path)
        renamed_files.append(str(new_path))

    return renamed_files


def un_make_tmp(files: list[str] | str) -> list[str]:
    """Un-hide files.

    Remove `.` from name and rename on disk.
    """
    files = utils._to_list(files)
    renamed_files = []

    for f in files:
        path = Path(f)
        new_path = path.with_name(path.name.removeprefix("."))
        path.rename(new_path)
        renamed_files.append(str(new_path))

    return renamed_files


@contextmanager
def tmp_renamed_files(remage_files):
    """Temporarily rename files, restoring originals on error and deleting temps on success."""
    originals = make_tmp(remage_files)
    try:
        yield originals
    except Exception:
        un_make_tmp(originals)
        raise
    else:
        for f in originals:
            Path(f).unlink()
