"""Verify data conservation between the flat and post-processed LH5 outputs.

For each detector table present in both files this checks:
  - the reshaped per-hit VoV fields, when flattened, contain the same multiset
    of values as the original flat table (no data lost or duplicated);
  - the derived `t0` column equals the first ``time`` of each hit group;
  - the number of `evtid` entries matches the number of hit groups, and the
    set of evtids appearing in the post-processed output equals the set in
    the input.
"""

from __future__ import annotations

import sys

import awkward as ak
import lh5
import numpy as np

flat_file = sys.argv[1]
pproc_file = sys.argv[2]


def _list_stp_tables(file: str) -> set[str]:
    # all tables under /stp/, excluding internal groups like /stp/__by_uid__
    return {
        t for t in lh5.ls(file, "stp/*") if not t.removeprefix("stp/").startswith("_")
    }


def _check_flat_table(tbl, flat, pproc, flat_fields, pproc_fields):
    """Check a calorimeter-like table forwarded as flat hits.

    These already hold one hit per detector per event, so the only change is
    the ``time`` -> ``t0`` rename; every field must be conserved.
    """
    checks = 0
    for f in flat_fields:
        target = "t0" if f == "time" else f
        if target not in pproc_fields:
            msg = f"{tbl}: field {f} (-> {target}) missing from post-processed output"
            raise RuntimeError(msg)
        flat_arr = np.sort(np.asarray(flat[f]))
        pproc_arr = np.sort(np.asarray(pproc[target]))
        if flat_arr.shape != pproc_arr.shape or not np.array_equal(flat_arr, pproc_arr):
            msg = f"{tbl}: flat field {f} values differ between flat and post-processed"
            raise RuntimeError(msg)
        checks += 1
    return checks


flat_tables = _list_stp_tables(flat_file)
pproc_tables = _list_stp_tables(pproc_file)
common = sorted(flat_tables & pproc_tables)

if not common:
    # no reshaped detector tables present (e.g. --no-stp runs): nothing to check
    print("verify_lh5_pproc: no stp/* tables present, skipping")
    sys.exit(0)

n_checks = 0
for tbl in common:
    flat = lh5.read(tbl, flat_file).view_as("ak", with_units=False)
    pproc = lh5.read(tbl, pproc_file).view_as("ak", with_units=False)

    flat_fields = set(flat.fields)
    pproc_fields = set(pproc.fields)

    # calorimeter-like tables are forwarded as flat hits (one row per detector
    # per event, only time -> t0): no reshaped VoV field is present
    if all(pproc[f].ndim == 1 for f in pproc_fields):
        n_checks += _check_flat_table(tbl, flat, pproc, flat_fields, pproc_fields)
        continue

    missing = flat_fields - pproc_fields
    if missing:
        msg = f"{tbl}: fields {missing} missing from post-processed output"
        raise RuntimeError(msg)

    for f in flat_fields:
        flat_col = flat[f]
        pproc_col = pproc[f]

        if pproc_col.ndim == 2:
            flattened = ak.flatten(pproc_col)
        elif pproc_col.ndim == 1 and f == "evtid":
            # evtid becomes one scalar per hit-group: compare unique sets
            if set(np.asarray(flat_col).tolist()) != set(
                np.asarray(pproc_col).tolist()
            ):
                msg = f"{tbl}: evtid set differs between flat and post-processed"
                raise RuntimeError(msg)
            continue
        else:
            msg = f"{tbl}: field {f} is unexpectedly scalar in post-processed output"
            raise RuntimeError(msg)

        flat_arr = np.sort(np.asarray(flat_col))
        pproc_arr = np.sort(np.asarray(flattened))
        if flat_arr.shape != pproc_arr.shape:
            msg = (
                f"{tbl}: field {f} length mismatch: "
                f"flat={flat_arr.shape}, pproc={pproc_arr.shape}"
            )
            raise RuntimeError(msg)
        if not np.array_equal(flat_arr, pproc_arr):
            msg = f"{tbl}: field {f} values differ between flat and post-processed"
            raise RuntimeError(msg)
        n_checks += 1

    # t0 must equal the first time of each hit group
    if "time" in pproc_fields and "t0" in pproc_fields:
        first_time = ak.fill_none(ak.firsts(pproc["time"], axis=-1), 0)
        if not np.array_equal(np.asarray(first_time), np.asarray(pproc["t0"])):
            msg = f"{tbl}: t0 does not equal first time of each hit group"
            raise RuntimeError(msg)
        n_checks += 1

    # evtid (one per group) must have the same length as the number of groups
    if "evtid" in pproc_fields and pproc["evtid"].ndim == 1 and "time" in pproc_fields:
        n_groups = len(ak.num(pproc["time"], axis=-1))
        if n_groups != len(pproc["evtid"]):
            msg = (
                f"{tbl}: evtid length {len(pproc['evtid'])} != "
                f"number of hit groups {n_groups}"
            )
            raise RuntimeError(msg)
        n_checks += 1

if n_checks == 0:
    msg = "no conservation checks performed -- something is off"
    raise RuntimeError(msg)

print(f"verify_lh5_pproc: {len(common)} detector tables, {n_checks} checks OK")
