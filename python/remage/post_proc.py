from __future__ import annotations

import copy
import logging
import time
from collections.abc import Mapping, Sequence
from contextlib import contextmanager
from pathlib import Path

import h5py
import numpy as np
import pygama.evt
import reboost
from lgdo import Array, Scalar, Struct, lh5
from lgdo.lh5.concat import lh5concat

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
    main_output_file: str = ipc_info.get_single("output_main", None)
    overwrite_output: bool = ipc_info.get_single("overwrite_output", "0") == "1"
    det_tables_path: str = ipc_info.get_single("ntuple_output_directory", None)

    ipc_info.remove("output_main")

    if main_output_file is None:
        return

    assert det_tables_path is not None

    output_file_exts = {
        Path(p).suffix.lower() for p in [*remage_files, main_output_file]
    }

    # these are mappings: <output scheme name> -> [<table name 1>, <table name 2>, ...]
    detector_info = utils._icp_to_dict(ipc_info.get("output_ntuple", 2))
    detector_info_aux = utils._icp_to_dict(ipc_info.get("output_ntuple_aux", 2))

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

        # extract the additional tables in the output file (not detectors)

        with tmp_renamed_files(remage_files) as original_files:
            # also get the additional tables to forward
            config = get_reboost_config(
                detector_info,
                detector_info_aux,
                time_window=time_window_in_us,
            )
            msg = f"Reboost config: {config}"
            log.debug(msg)

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

            # also copy __by_uid__ group to the output files
            # we do this here and not in reboost, as the links require special syntax
            # NOTE: using just the first original file since the links are always the same
            copy_links(original_files[0], output_files, lh5_links_group_name)

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
                exclude_list=[f"{lh5_links_group_name}/*"],
            )
            # also copy __by_uid__ group to the output files
            # we do this here and not in reboost, as lh5concat does not copy links correctly.
            # NOTE: using just the first original file since the links are always the same
            copy_links(original_files[0], main_output_file, lh5_links_group_name)

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
            obj = (
                Struct({col: Scalar(table[col].nda[idx]) for col in keys})
                if len(keys) > 1
                else Scalar(table[keys[0]].nda[idx])
            )
            d[table[unique_col].nda[idx].decode("utf-8")] = obj

        lh5.write(Struct(d), table_name, file, wo_mode="overwrite")
    else:
        lh5.write(table, table_name, file, wo_mode="overwrite")


def get_reboost_config(
    detector_info: Mapping[str, Sequence[str]],
    detector_info_aux: Mapping[str, Sequence[str]],
    *,
    time_window: float = 10,
) -> dict:
    """Get the config file to run reboost.

    Parameters
    ----------
    detector_info
        a mapping of tables in the remage file that need to be reshaped, keyed
        by output scheme name (i.e. RMGGermaniumOutputScheme).
    detector_info_aux
        same as `detector_info`, but holds non-standard tables.
    time_window
        time window to use for building hits (in us).

    Returns
    -------
    config file as a dictionary.
    """
    config = {}

    # build a default config for all tables (exclude calorimeter tables)
    table_list = [
        v
        for k, vals in detector_info.items()
        if k != "RMGCalorimeterOutputScheme"
        for v in vals
    ]

    default_config = {
        "name": "default",
        "detector_mapping": [{"output": table} for table in table_list],
        "hit_table_layout": f"reboost.shape.group.group_by_time(STEPS, {time_window})",
        "operations": {
            "t0": {
                "expression": "ak.fill_none(ak.firsts(HITS.time, axis=-1), 0)",
                "units": "ns",
            },
            "evtid": "ak.fill_none(ak.firsts(HITS.evtid, axis=-1), 0)",
        },
    }
    config["processing_groups"] = [default_config]

    # special treatment for the calorimeter output scheme
    calo_config = copy.deepcopy(default_config)
    calo_config["name"] = "RMGCalorimeterOutputScheme"
    calo_config["detector_mapping"] = [
        {"output": table} for table in detector_info["RMGCalorimeterOutputScheme"]
    ]
    # there is always one energy per event, then remove one dimension from the array
    calo_config["operations"]["edep"] = "ak.ravel(HITS.edep)"

    config["processing_groups"].append(calo_config)

    # forward other tables as they are
    config["forward"] = [v for vals in detector_info_aux.values() for v in vals]

    return config


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
