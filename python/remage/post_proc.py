# Copyright (C) 2024 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

import logging
import multiprocessing
import os
import sys
import threading
import time
from concurrent.futures import ProcessPoolExecutor
from contextlib import contextmanager
from functools import partial
from pathlib import Path

import h5py
import lh5
import numpy as np
import pygama.evt
from lgdo import Array, Scalar, Struct
from lh5.io.concat import lh5concat

from . import utils
from .ipc import IpcResult
from .logging import setup_log
from .reshaping import reshape_output

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
    append_output: bool = ipc_info.get_single("append_output", "0") == "1"
    det_tables_path: str | None = ipc_info.get_single("ntuple_output_directory")
    # the whole output can be stored in a group instead of the file root
    output_group: str = ipc_info.get_single("output_group") or ""

    ipc_info.remove("output_main")

    if main_output_file is None:
        return

    assert det_tables_path is not None

    group_path = output_group + "/" if output_group else ""
    det_tables_path = group_path + det_tables_path

    output_file_exts = {
        Path(p).suffix.lower() for p in [*remage_files, main_output_file]
    }

    # these are mappings: <output scheme name> -> [<table name 1>, <table name 2>, ...]
    detector_info = ipc_info.get_as_dict("output_ntuple")
    detector_info_aux = ipc_info.get_as_dict("output_ntuple_aux")

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
    lh5_event_number_name: str = (
        "/" + group_path + ipc_info.get("lh5_event_number_name")[0]
    )

    time_start = time.time()

    # if merging is on, write everything to a single file
    output_files: list[str] | str = (
        remage_files if not merge_output_files else main_output_file
    )

    # the merged output file is written from scratch, so hide it as well to be able to keep
    # the objects it already contains
    pre_existing: list[str] = (
        [main_output_file]
        if merge_output_files
        and append_output
        and main_output_file not in remage_files
        and Path(main_output_file).exists()
        else []
    )

    if not flat_output:
        msg = (
            "Reshaping "
            + ("and merging " if merge_output_files else "")
            + "output files"
        )
        log.info(msg)

        # detector tables that get step-grouped into hits (Germanium,
        # Scintillator, Optical), deduplicated while preserving order: in the
        # single-table layout several detectors of the same type map to the same
        # output table, so the same name shows up multiple times.
        reshape_detectors = list(
            dict.fromkeys(
                table
                for scheme, tables in detector_info.items()
                if scheme != "RMGCalorimeterOutputScheme"
                for table in tables
            )
        )

        # calorimeter tables already hold one hit per detector per event and
        # must not be step-grouped; they are forwarded as flat hit tables
        flat_hit_detectors = list(
            dict.fromkeys(detector_info.get("RMGCalorimeterOutputScheme", []))
        )

        # additional (non-detector) tables in the output file, forwarded as-is
        extra_tables = list(
            dict.fromkeys(
                group_path + table
                for tables in detector_info_aux.values()
                for table in tables
            )
        )

        with (
            tmp_renamed_files(remage_files) as original_files,
            tmp_renamed_files(pre_existing) as pre_existing_files,
        ):
            run = partial(
                _reshape_and_build_tcm,
                reshape_tables=reshape_detectors,
                forward_tables=extra_tables,
                flat_hit_tables=flat_hit_detectors,
                det_tables_path=det_tables_path,
                group_path=group_path,
                lh5_links_group_name=lh5_links_group_name,
                lh5_event_number_name=lh5_event_number_name,
                time_window_in_us=time_window_in_us,
                overwrite=overwrite_output,
                append=append_output,
            )

            mp_context = None
            if (
                not merge_output_files
                and len(original_files) > 1
                and os.getenv("RMG_DISABLE_PARALLEL_PPROC") != "1"
            ):
                mp_context = _pool_mp_context()

            if mp_context is None:
                run(original_files, pre_existing_files, output_files)
            else:
                # each input file has its own output file, so the files can be
                # processed at the same time, each one by its own process
                # remage-cpp writes one output file per worker thread/process,
                # so the number of files is the parallelism the user asked for.
                with ProcessPoolExecutor(
                    initializer=_setup_worker_log,
                    initargs=(log.level,),
                    max_workers=min(len(original_files), _available_cpus()),
                    mp_context=mp_context,
                ) as pool:
                    futures = [
                        pool.submit(run, [f_in], [], f_out)
                        for f_in, f_out in zip(
                            original_files, output_files, strict=True
                        )
                    ]
                    for future in futures:
                        future.result()

        # set the output file(s) for downstream consumers.
        ipc_info.set("output", output_files)

    if flat_output and merge_output_files:
        msg = "Merging output files"
        log.info(msg)

        with (
            tmp_renamed_files(remage_files) as original_files,
            tmp_renamed_files(pre_existing) as pre_existing_files,
        ):
            lh5concat(
                lh5_files=original_files,
                output=main_output_file,
                overwrite=overwrite_output,
                exclude_list=[
                    f"{lh5_links_group_name}/*",
                    f"{lh5_event_number_name}/*",
                    lh5_event_number_name,
                ],
            )
            # also copy __by_uid__ group to the output files
            # we do this here and not in reboost, as lh5concat does not copy links correctly.
            # NOTE: using just the first original file since the links are always the same
            copy_links(original_files[0], main_output_file, lh5_links_group_name)
            update_number_of_simulated_events(
                original_files, output_files, lh5_event_number_name
            )
            if append_output:
                copy_untouched_objects(
                    [*pre_existing_files, *original_files], main_output_file
                )

        ipc_info.set("output", main_output_file)

    # deduplicate entries of the process table.
    ntuples_to_deduplicate = set(ipc_info.get("output_ntuple_deduplicate"))
    for file in utils._to_list(output_files):
        for table in ntuples_to_deduplicate:
            deduplicate_table(file, group_path + table, "name", not flat_output)

    msg = f"Finished post-processing which took {int(time.time() - time_start)} s"
    log.info(msg)


def _available_cpus() -> int:
    """Number of CPUs usable by this process (respects affinity masks, e.g. under Slurm)."""
    if hasattr(os, "process_cpu_count"):  # Python >= 3.13
        return os.process_cpu_count() or 1
    if hasattr(os, "sched_getaffinity"):
        return len(os.sched_getaffinity(0)) or 1
    return os.cpu_count() or 1


#: set by the ``remage`` CLI entry point, whose main script only calls remage
#: behind an ``if __name__ == "__main__":`` guard.
_main_is_remage_cli = False


def _main_reimport_is_safe() -> bool:
    """Whether workers started with spawn/forkserver can re-import ``__main__``.

    Both start methods run the caller's main script again in every worker. For
    a script that calls remage at top level without an
    ``if __name__ == "__main__":`` guard, this would run the whole simulation
    again in each worker, so only allow it when the main module is known.
    """
    if _main_is_remage_cli:
        return True
    main = sys.modules.get("__main__")
    # interactive sessions (REPL, Jupyter) have no main script to re-import
    if getattr(main, "__file__", None) is None:
        return True
    # `python -m pkg` mains are never re-imported by multiprocessing
    spec_name = getattr(getattr(main, "__spec__", None), "name", None) or ""
    return spec_name == "__main__" or spec_name.endswith(".__main__")


def _pool_mp_context() -> multiprocessing.context.BaseContext | None:
    """Pick a start method for the post-processing workers.

    Returns ``None`` if the files should be processed sequentially instead.
    """
    methods = multiprocessing.get_all_start_methods()

    # fork is cheapest and does not re-import __main__, but is only safe in a
    # single-threaded process: another thread (e.g. in a Jupyter kernel) could
    # hold a lock at fork time that then stays locked forever in the child.
    # it is also unsafe on macOS, where system libraries are not fork-safe.
    if sys.platform != "darwin" and "fork" in methods and threading.active_count() == 1:
        return multiprocessing.get_context("fork")

    if _main_reimport_is_safe():
        return multiprocessing.get_context(
            "forkserver" if "forkserver" in methods else "spawn"
        )

    msg = (
        "post-processing output files sequentially: cannot start worker "
        "processes safely from this main script"
    )
    log.warning(msg)
    return None


def _setup_worker_log(level: int) -> None:
    """Make a worker process log like the main process."""
    setup_log().setLevel(level)


def _reshape_and_build_tcm(
    original_files: list[str],
    pre_existing_files: list[str],
    output_files: list[str] | str,
    *,
    reshape_tables: list[str],
    forward_tables: list[str],
    flat_hit_tables: list[str],
    det_tables_path: str,
    group_path: str,
    lh5_links_group_name: str,
    lh5_event_number_name: str,
    time_window_in_us: float,
    overwrite: bool,
    append: bool,
) -> None:
    """Turn step files into hit files and add the time-coincidence map."""
    # post-process outputs: reshape detector tables by time-grouping,
    # forward calorimeter tables as flat hits and auxiliary tables
    # unchanged
    reshape_output(
        stp_files=original_files,
        hit_files=output_files,
        reshape_tables=reshape_tables,
        forward_tables=forward_tables,
        flat_hit_tables=flat_hit_tables,
        out_field=det_tables_path,
        time_window_in_us=time_window_in_us,
        overwrite=overwrite,
    )

    # also copy __by_uid__ group to the output files
    # we do this here and not in reboost, as the links require special syntax
    # NOTE: using just the first original file since the links are always the same
    copy_links(original_files[0], output_files, lh5_links_group_name)
    update_number_of_simulated_events(
        original_files, output_files, lh5_event_number_name
    )

    # add a time-coincidence map to the output file(s)
    msg = f"Computing and storing the TCM as /{group_path}tcm"
    log.info(msg)

    for file in utils._to_list(output_files):
        # do not compute the TCM if there are no stepping tables
        if lh5.ls(file, rf"{lh5_links_group_name}/det*") != []:
            # use tables keyed by UID in the __by_uid__ group.  in this way, the
            # TCM will index tables by UID.  the coincidence criterium is based
            # on Geant4 event identifier and time of the hits
            # NOTE: uses the same time window as in reshape_output() reshaping
            pygama.evt.build_tcm(
                [(file, rf"{lh5_links_group_name}/*")],  # input_tables
                ["evtid", "t0"],  # coin_cols
                hash_func=rf"(?<={lh5_links_group_name}/det)\d+",
                coin_windows=[0, time_window_in_us * 1000],
                out_file=file,
                out_name=group_path + "tcm",
                wo_mode="write_safe",
            )

    if append:
        copy_untouched_objects([*pre_existing_files, *original_files], output_files)


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


def copy_untouched_objects(
    original_files: list[str], output_files: str | list[str]
) -> None:
    """Copy the objects that the current run did not write to the output files.

    Objects that are already in an output file are left untouched, i.e. the new data wins over
    the data that was already in the file.
    """
    output_files = utils._to_list(output_files)
    original_files = utils._to_list(original_files)

    # we can either handle the case of one single output file, or the same number of input/output files.
    if len(output_files) != len(original_files):
        assert len(output_files) == 1
        output_files = output_files * len(original_files)

    for f_in, f_out in zip(original_files, output_files, strict=True):
        with h5py.File(f_in, "r") as inf, h5py.File(f_out, "a") as ouf:
            for name in inf:
                if name in ouf:
                    continue
                msg = f"copying {name} from {f_in} to {f_out}"
                log.debug(msg)
                inf.copy(
                    name,
                    ouf,
                    name,
                    expand_soft=False,  # do _not_ follow soft-links; preserve them
                    expand_external=False,  # likewise for external links
                    expand_refs=False,  # likewise for object-reference datasets
                )


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
