from __future__ import annotations

import logging
from pathlib import Path

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


def make_tmp(files: list[str] | str) -> list[str]:
    """Append files with a '.' so they can be overwritten."""

    if isinstance(files, str):
        files = [files]

    renamed_files = []

    for f in files:
        path = Path(f)
        new_path = path.with_name("." + path.name)
        path.rename(new_path)
        renamed_files.append(str(new_path))

    return renamed_files
