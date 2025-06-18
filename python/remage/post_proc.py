from __future__ import annotations

import logging
import time
from collections.abc import Sequence
from contextlib import contextmanager
from pathlib import Path

import pygama.evt
import reboost
from lgdo import lh5
from lgdo.lh5.concat import lh5concat

from .ipc import IpcResult

log = logging.getLogger("remage")


def post_proc(
    ipc_info: IpcResult,
    flat_output: bool,
    merge_output_files: bool,
    time_window_in_us: float,
) -> None:
    remage_files = ipc_info.get("output")
    main_output_file = ipc_info.get_single("output_main", None)
    overwrite_output = ipc_info.get_single("overwrite_output", "0") == "1"
    det_tables_path = ipc_info.get_single("ntuple_output_directory", None)

    ipc_info.remove("output_main")

    if main_output_file is None:
        return

    assert det_tables_path is not None

    output_file_exts = {
        Path(p).suffix.lower() for p in [*remage_files, main_output_file]
    }

    detector_info = ipc_info.get("output_table", 2)

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

    time_start = time.time()

    if not flat_output:
        # if merging is on, write everything to a single file
        output_files = remage_files if not merge_output_files else main_output_file

        msg = (
            "Reshaping "
            + ("and merging " if merge_output_files else "")
            + "output files"
        )
        log.info(msg)

        # registered scintillator or germanium detectors
        registered_detectors = list(
            {
                det[1]
                for det in detector_info
                if det[0] == "germanium" or det[0] == "scintillator"
            }
        )

        # extract the additional tables in the output file (not detectors)
        extra_detectors = []
        for table in lh5.ls(remage_files[0], lh5_group=f"{det_tables_path}/"):
            name = table.split("/")[1]
            if name not in registered_detectors:
                extra_detectors.append(table)

        # add the vertex table, if it was stored
        extra_tables = list(set(extra_detectors + ipc_info.get("vtx_table_path")))

        with tmp_renamed_files(remage_files) as original_files:
            # also get the additional tables to forward
            config = get_reboost_config(
                registered_detectors,
                extra_tables,
                time_window=time_window_in_us,
            )

            # use reboost to post-process outputs
            reboost.build_hit(
                config,
                {},
                stp_files=original_files,
                glm_files=None,
                hit_files=output_files,
                out_field=det_tables_path,
                overwrite=overwrite_output,
            )

            # make the tcm
            msg = "Computing and storing the TCM as /tcm"
            log.info(msg)

        # add a time-coincidence map to the output file(s)
        _tcm_infiles = output_files
        if not isinstance(output_files, list | tuple):
            _tcm_infiles = [output_files]

        tables = lh5.ls(_tcm_infiles[0], lh5_group=f"{det_tables_path}/")

        for file in _tcm_infiles:
            pygama.evt.build_tcm(
                [(file, tab) for tab in tables],  # input_tables
                ["evtid", "t0"],  # coin_cols
                hash_func=None,
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
            )

        ipc_info.set("output", main_output_file)

    msg = f"Finished post-processing which took {int(time.time() - time_start)} s"
    log.info(msg)


def get_reboost_config(
    reshape_table_list: Sequence[str],
    other_table_list: Sequence[str],
    *,
    time_window: float = 10,
) -> dict:
    """Get the config file to run reboost.

    Parameters
    ----------
    reshape_table_list
        a list of the table in the remage file that need to be reshaped
        (i.e. Germanium or Scintillator output)
    other_table_list
        other tables in the file.
    time_window
        time window to use for building hits (in us).

    Returns
    -------
    config file as a dictionary.
    """
    config = {}

    # get the config for tables to be reshaped
    config["processing_groups"] = [
        {
            "name": "all",
            "detector_mapping": [{"output": table} for table in reshape_table_list],
            "hit_table_layout": f"reboost.shape.group.group_by_time(STEPS, {time_window})",
            "operations": {
                "t0": {
                    "expression": "ak.fill_none(ak.firsts(HITS.time, axis=-1), 0)",
                    "units": "ns",
                },
                "evtid": "ak.fill_none(ak.firsts(HITS.evtid, axis=-1), 0)",
            },
        }
    ]

    # forward other tables as they are
    config["forward"] = other_table_list

    return config


def make_tmp(files: list[str] | str) -> list[str]:
    """Hide files.

    Prepend a `.` to their name and rename them on disk.
    """

    if isinstance(files, str):
        files = [files]

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
    if isinstance(files, str):
        files = [files]

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
