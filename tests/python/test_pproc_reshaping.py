from __future__ import annotations

import copy

import awkward as ak
import lh5
import numpy as np
import pytest
from lgdo import Array, Table, VectorOfVectors
from remage.reshaping import reshape_output


@pytest.fixture(scope="module")
def stp_file(tmptestdir):
    """A minimal remage-cpp-like step file with two detectors and a vertex table.

    Step tables are *flat* (one row per step) as written by remage; ``evtid``
    and ``time`` (in ns) drive the time-grouping into hits.
    """
    stp_path = str(tmptestdir / "basic.lh5")

    data = {}
    data["evtid"] = Array([0, 0, 1, 1, 1])
    data["edep"] = Array([100, 200, 10, 20, 300], attrs={"units": "keV"})  # keV
    data["time"] = Array([0, 1.5, 0.1, 2.1, 3.7], attrs={"units": "ns"})  # ns
    data["xloc"] = Array([0.01, 0.02, 0.001, 0.003, 0.005], attrs={"units": "m"})  # m
    data["yloc"] = Array([0.01, 0.02, 0.001, 0.003, 0.005], attrs={"units": "m"})  # m
    data["zloc"] = Array([0.04, 0.02, 0.001, 0.023, 0.005], attrs={"units": "m"})  # m
    data["dist_to_surf"] = Array(
        [0.04, 0.02, 0.011, 0.003, 0.051], attrs={"units": "m"}
    )  # m

    tab = Table(data)
    tab2 = copy.deepcopy(tab)

    lh5.write(tab, "stp/det1", stp_path, wo_mode="of")
    lh5.write(tab2, "stp/det2", stp_path, wo_mode="append")
    lh5.write(Table({"evtid": Array([0, 1])}), "vtx", stp_path, wo_mode="append")

    return stp_path


def test_reshape(stp_file, tmptestdir):
    outfile = f"{tmptestdir}/basic_hit_reshaped.lh5"

    reshape_output(
        stp_files=[stp_file],
        hit_files=outfile,
        reshape_tables=["det1", "det2"],
        forward_tables=["vtx"],
        out_field="stp",
        time_window_in_us=10,
        overwrite=True,
    )

    assert lh5.ls(outfile) == ["stp", "vtx"]

    output = lh5.read("stp/det1", outfile)
    out_ak = output.view_as("ak")

    # the step-level (VectorOfVectors) fields keep their values, grouped by hit
    assert ak.all(out_ak.edep == [[100, 200], [10, 20, 300]])
    assert ak.all(out_ak.time == [[0, 1.5], [0.1, 2.1, 3.7]])
    assert ak.all(out_ak.xloc == [[0.01, 0.02], [0.001, 0.003, 0.005]])
    assert ak.all(out_ak.yloc == [[0.01, 0.02], [0.001, 0.003, 0.005]])
    assert ak.all(out_ak.zloc == [[0.04, 0.02], [0.001, 0.023, 0.005]])
    assert ak.all(out_ak.dist_to_surf == [[0.04, 0.02], [0.011, 0.003, 0.051]])

    # per-hit scalar columns added by the reshaping
    assert ak.all(out_ak.evtid == [0, 1])
    assert ak.all(out_ak.t0 == [0, 0.1])

    # det2 is identical to det1 (same input data)
    assert lh5.read("stp/det2", outfile).view_as("ak").edep.to_list() == [
        [100, 200],
        [10, 20, 300],
    ]

    # the vtx table is forwarded unchanged
    assert lh5.read("vtx", outfile) == Table({"evtid": Array([0, 1])})

    # units are moved onto the flattened_data of the VoV fields
    for field, unit in zip(
        ["edep", "time", "xloc", "yloc", "zloc", "dist_to_surf"],
        ["keV", "ns", "m", "m", "m", "m"],
        strict=True,
    ):
        assert "units" not in output[field].attrs
        assert output[field].flattened_data.attrs["units"] == unit

    # t0 is a flat Array carrying its unit directly
    assert output["t0"].attrs["units"] == "ns"


def test_only_forward(stp_file, tmptestdir):
    outfile = f"{tmptestdir}/basic_hit_forward_only.lh5"

    for _ in range(2):  # test that overwriting the output file works.
        reshape_output(
            stp_files=[stp_file],
            hit_files=outfile,
            reshape_tables=[],
            forward_tables=["vtx"],
            out_field="stp",
            time_window_in_us=10,
            overwrite=True,
        )

    assert lh5.read("vtx", outfile) == Table({"evtid": Array([0, 1])})


def test_file_merging(stp_file, tmptestdir):
    outfile = f"{tmptestdir}/basic_hit_merged.lh5"

    # two input files merged into a single output file
    reshape_output(
        stp_files=[stp_file, stp_file],
        hit_files=outfile,
        reshape_tables=["det1", "det2"],
        forward_tables=["vtx"],
        out_field="stp",
        time_window_in_us=10,
        overwrite=True,
    )

    assert lh5.ls(outfile) == ["stp", "vtx"]

    # the hits from both input files are concatenated
    assert len(lh5.read("stp/det1", outfile).view_as("ak")) == 4
    assert len(lh5.read("stp/det2", outfile).view_as("ak")) == 4


def test_multi_file(stp_file, tmptestdir):
    outfiles = [f"{tmptestdir}/basic_hit_t0.lh5", f"{tmptestdir}/basic_hit_t1.lh5"]

    # two input files written to two separate output files
    reshape_output(
        stp_files=[stp_file, stp_file],
        hit_files=outfiles,
        reshape_tables=["det1", "det2"],
        forward_tables=["vtx"],
        out_field="stp",
        time_window_in_us=10,
        overwrite=True,
    )

    for file in outfiles:
        assert lh5.ls(file) == ["stp", "vtx"]
        assert len(lh5.read("stp/det1", file).view_as("ak")) == 2


def test_overwrite(stp_file, tmptestdir):
    outfile = f"{tmptestdir}/basic_hit_overwrite.lh5"

    # write once, then overwrite with a merge of two input files; the second
    # run must not leave stale rows from the first behind
    reshape_output(
        stp_files=[stp_file],
        hit_files=outfile,
        reshape_tables=["det1"],
        forward_tables=["vtx"],
        out_field="stp",
        time_window_in_us=10,
        overwrite=True,
    )
    assert len(lh5.read("stp/det1", outfile).view_as("ak")) == 2

    reshape_output(
        stp_files=[stp_file, stp_file],
        hit_files=outfile,
        reshape_tables=["det1"],
        forward_tables=["vtx"],
        out_field="stp",
        time_window_in_us=10,
        overwrite=True,
    )
    assert len(lh5.read("stp/det1", outfile).view_as("ak")) == 4


def test_unit_convention_matches_manual_grouping(stp_file, tmptestdir):
    """The on-disk units follow the move-to-flattened_data convention."""
    outfile = f"{tmptestdir}/basic_hit_units.lh5"

    reshape_output(
        stp_files=[stp_file],
        hit_files=outfile,
        reshape_tables=["det1"],
        forward_tables=["vtx"],
        out_field="stp",
        time_window_in_us=10,
        overwrite=True,
    )

    edep = lh5.read("stp/det1/edep", outfile)
    assert isinstance(edep, VectorOfVectors)
    assert "units" not in edep.attrs
    assert edep.flattened_data.attrs["units"] == "keV"


@pytest.fixture(scope="module")
def big_stp_file(tmptestdir):
    """A larger step file spanning many events with a few steps each.

    Used to exercise the chunked (multi-buffer) reshaping path: with a small
    ``buffer`` the iterator splits the table across several chunks, each grouped
    and appended independently.
    """
    stp_path = str(tmptestdir / "big.lh5")
    rng = np.random.default_rng(0)

    n_events = 500
    # 1-4 steps per event, contiguous evtids as remage writes them
    counts = rng.integers(1, 5, size=n_events)
    evtid = np.repeat(np.arange(n_events), counts)
    n = len(evtid)
    # times within an event spread across/within the coincidence window (ns)
    time = np.cumsum(rng.uniform(0, 4000, size=n)).astype(float)

    data = {
        "evtid": Array(evtid),
        "edep": Array(rng.uniform(0, 1000, size=n), attrs={"units": "keV"}),
        "time": Array(time, attrs={"units": "ns"}),
    }
    lh5.write(Table(data), "stp/det1", stp_path, wo_mode="of")
    lh5.write(
        Table({"evtid": Array(np.arange(n_events))}), "vtx", stp_path, wo_mode="append"
    )

    return stp_path


@pytest.mark.parametrize("buffer", [7, 13, 100, int(5e6)])
def test_chunked_reshape_conserves_data(big_stp_file, tmptestdir, buffer):
    """Reshaping is independent of ``buffer``: no step is lost or duplicated.

    A small buffer forces many event-aligned chunks, each written with an
    ``append`` mode. The reshaped hit table must be identical regardless of
    buffer size, and must contain exactly the input steps (same multiset).
    """
    outfile = f"{tmptestdir}/big_hit_buf{buffer}.lh5"

    reshape_output(
        stp_files=[big_stp_file],
        hit_files=outfile,
        reshape_tables=["det1"],
        forward_tables=["vtx"],
        out_field="stp",
        time_window_in_us=1,
        overwrite=True,
        buffer=buffer,
    )

    flat_in = lh5.read("stp/det1", big_stp_file).view_as("ak")
    hits = lh5.read("stp/det1", outfile).view_as("ak")

    # every step value is conserved (no loss, no duplication) for each field
    for field in ("edep", "time"):
        np.testing.assert_array_equal(
            np.sort(ak.flatten(hits[field]).to_numpy()),
            np.sort(flat_in[field].to_numpy()),
        )

    # evtids are conserved as a set, and t0 is the first time of each hit group
    assert set(hits.evtid.to_list()) == set(flat_in.evtid.to_list())
    np.testing.assert_array_equal(
        hits.t0.to_numpy(), ak.firsts(hits.time, axis=-1).to_numpy()
    )

    # the full hit structure is byte-for-byte independent of the buffer size
    ref_file = f"{tmptestdir}/big_hit_ref.lh5"
    reshape_output(
        stp_files=[big_stp_file],
        hit_files=ref_file,
        reshape_tables=["det1"],
        forward_tables=["vtx"],
        out_field="stp",
        time_window_in_us=1,
        overwrite=True,
        buffer=int(5e6),
    )
    assert hits.to_list() == lh5.read("stp/det1", ref_file).view_as("ak").to_list()


def test_nonmonotonic_evtids_raises(tmptestdir):
    """A non-monotonic evtid column (event split across blocks) is rejected loudly."""
    stp_path = str(tmptestdir / "noncontig.lh5")
    # evtid 0 reappears after evtid 1 - not monotonically increasing
    lh5.write(
        Table(
            {
                "evtid": Array([0, 0, 1, 0]),
                "time": Array([0.0, 1.0, 2.0, 3.0], attrs={"units": "ns"}),
            }
        ),
        "stp/det1",
        stp_path,
        wo_mode="of",
    )

    with pytest.raises(ValueError, match="not monotonically increasing"):
        reshape_output(
            stp_files=[stp_path],
            hit_files=f"{tmptestdir}/noncontig_out.lh5",
            reshape_tables=["det1"],
            forward_tables=[],
            out_field="stp",
            time_window_in_us=1,
            overwrite=True,
        )


@pytest.fixture(scope="module")
def calo_stp_file(tmptestdir):
    """A step file with a calorimeter table already in per-hit (flat) form.

    The calorimeter scheme accumulates one hit (total edep) per detector per
    event in the C++ stage, so its table is flat: one row per detector per
    event, with ``det_uid`` as in the single-table layout.
    """
    stp_path = str(tmptestdir / "calo.lh5")

    lh5.write(
        Table(
            {
                "evtid": Array([0, 0, 1]),
                "det_uid": Array([1001, 1002, 1001]),
                "edep": Array([10.0, 20.0, 30.0], attrs={"units": "keV"}),
                "time": Array([0.5, 1.5, 2.5], attrs={"units": "ns"}),
            }
        ),
        "stp/calorimeter",
        stp_path,
        wo_mode="of",
    )
    # a reshaped detector alongside, to exercise mixed reshape + flat output
    lh5.write(
        Table(
            {
                "evtid": Array([0, 0, 1]),
                "edep": Array([100.0, 200.0, 300.0], attrs={"units": "keV"}),
                "time": Array([0.0, 1.0, 2.0], attrs={"units": "ns"}),
            }
        ),
        "stp/det1",
        stp_path,
        wo_mode="append",
    )
    lh5.write(Table({"evtid": Array([0, 1])}), "vtx", stp_path, wo_mode="append")

    return stp_path


def test_flat_hit_table_calorimeter(calo_stp_file, tmptestdir):
    outfile = f"{tmptestdir}/calo_hit.lh5"

    reshape_output(
        stp_files=[calo_stp_file],
        hit_files=outfile,
        reshape_tables=["det1"],
        forward_tables=["vtx"],
        flat_hit_tables=["calorimeter"],
        out_field="stp",
        time_window_in_us=10,
        overwrite=True,
    )

    assert lh5.ls(outfile) == ["stp", "vtx"]

    calo = lh5.read("stp/calorimeter", outfile)
    calo_ak = calo.view_as("ak")

    # the calorimeter table stays flat (not step-grouped): one row per input row
    assert calo_ak.evtid.to_list() == [0, 0, 1]
    assert calo_ak.det_uid.to_list() == [1001, 1002, 1001]
    assert calo_ak.edep.to_list() == [10.0, 20.0, 30.0]

    # time is renamed to t0 (and only that); no `time` column remains
    assert "time" not in calo_ak.fields
    assert calo_ak.t0.to_list() == [0.5, 1.5, 2.5]

    # units are forwarded unchanged (flat Arrays keep units directly)
    assert calo["edep"].attrs["units"] == "keV"
    assert calo["t0"].attrs["units"] == "ns"

    # the reshaped detector alongside is still grouped into hits
    det1 = lh5.read("stp/det1", outfile).view_as("ak")
    assert det1.edep.to_list() == [[100.0, 200.0], [300.0]]
    assert det1.t0.to_list() == [0.0, 2.0]
