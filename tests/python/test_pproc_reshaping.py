from __future__ import annotations

import copy

import awkward as ak
import lh5
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
