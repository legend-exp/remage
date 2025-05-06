from __future__ import annotations

import logging
from contextlib import contextmanager
from pathlib import Path

from lgdo import lh5

log = logging.getLogger(__name__)


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

    config = {"processing_groups": []}

    # get the config for tables to be reshaped
    reshape_tables = {
        "name": "all",
        "detector_mapping": [{"output": table} for table in reshape_table_list],
        "hit_table_layout": f"reboost.shape.group.group_by_time(STEPS, {time_window})",
    }
    config["processing_groups"].append(reshape_tables)

    for other in other_table_list:
        config["processing_groups"].append(
            {
                "name": other,
                "detector_mapping": [
                    {"output": other},
                ],
            }
        )

    return config


def get_extra_tables(file: str, detectors: list[str]) -> list[str]:
    """Extract the additional tables in the output file (not detectors)."""

    tables = lh5.ls(file, lh5_group="stp/")

    return [tab.split("/")[1] for tab in tables if tab.split("/")[1] not in detectors]


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
