from __future__ import annotations

import logging
import time
from contextlib import contextmanager
from pathlib import Path

from lgdo import lh5
from lgdo.lh5.concat import lh5concat
from reboost.build_hit import build_hit

from .ipc import IpcResult

log = logging.getLogger(__name__)


def post_proc(
    ipc_info: IpcResult,
    flat_output: bool,
    merge_output_files: bool,
    time_window_in_us: float,
) -> None:
    remage_files = ipc_info.get("output")
    main_output_file = ipc_info.get_single("output_main", None)
    overwrite_output = ipc_info.get_single("overwrite_output", "0") == "1"

    ipc_info.remove("output_main")

    if main_output_file is None:
        return

    output_file_exts = {
        Path(p).suffix.lower() for p in [*remage_files, main_output_file]
    }

    detector_info = ipc_info.get("output_table", 2)

    assert len(output_file_exts) == 1

    if output_file_exts != {".lh5"}:
        if not flat_output or merge_output_files:
            log.error(
                "merging or reshaping is not supported for output format %s",
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
        output_files = remage_files if merge_output_files else main_output_file

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

        with tmp_renamed_files(remage_files) as original_files:
            # also get the additional tables to forward
            config = get_rebooost_config(
                registered_detectors,
                get_extra_tables(original_files[0], registered_detectors),
                time_window=time_window_in_us,
            )

            # use reboost to post-process outputs
            build_hit(
                config,
                {},
                stp_files=original_files,
                glm_files=None,
                hit_files=output_files,
                out_field="stp",
            )

        # set the merged output file for downstream consumers.
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


def get_rebooost_config(
    reshape_table_list: list[str],
    other_table_list: list[str],
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
        }
    ]

    # forward other tables as they are
    config["forward"] = other_table_list

    return config


def get_extra_tables(file: str, detectors: list[str]) -> list[str]:
    """Extract the additional tables in the output file (not detectors)."""

    extra_detectors = []
    for table in lh5.ls(file, lh5_group="stp/"):
        name = table.split("/")[1]
        if name not in detectors:
            extra_detectors.append(table)

    other_tables = ["vtx"]

    return extra_detectors + other_tables


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
