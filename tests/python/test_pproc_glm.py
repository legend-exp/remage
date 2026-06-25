from __future__ import annotations

import awkward as ak
import lh5
import numpy as np
import pytest
from lgdo import Table
from remage.reshaping import _iter_event_aligned_chunks


@pytest.fixture(scope="module")
def stp_files(tmptestdir):
    rng = np.random.default_rng(0)

    # simple: every evtid present in vertices
    simple = tmptestdir / "simple_test.lh5"
    lh5.write(Table(ak.Array({"evtid": np.arange(10000)})), "vtx", simple, wo_mode="of")
    lh5.write(
        Table(ak.Array({"evtid": np.sort(rng.integers(0, 10000, size=21082))})),
        "stp/det1",
        simple,
        wo_mode="append",
    )
    lh5.write(
        Table(ak.Array({"evtid": np.sort(rng.integers(0, 1000, size=1069))})),
        "stp/det2",
        simple,
        wo_mode="append",
    )

    # gaps: evtids skip (multithreaded-like)
    gaps = tmptestdir / "gaps_test.lh5"
    vert = np.sort(np.unique(rng.integers(0, 200000, size=10000)))
    lh5.write(Table(ak.Array({"evtid": vert})), "vtx", gaps, wo_mode="of")
    lh5.write(
        Table(ak.Array({"evtid": np.sort(rng.choice(vert, size=21082))})),
        "stp/det1",
        gaps,
        wo_mode="append",
    )
    lh5.write(
        Table(ak.Array({"evtid": np.sort(rng.choice(vert, size=1069))})),
        "stp/det2",
        gaps,
        wo_mode="append",
    )

    return {"simple": str(simple), "gaps": str(gaps)}


@pytest.mark.parametrize("test", ["simple", "gaps"])
@pytest.mark.parametrize("det", ["det1", "det2"])
@pytest.mark.parametrize("buffer", [50, 5000, 100000])
def test_iter_event_aligned_chunks(stp_files, test, det, buffer):
    stp_file = stp_files[test]
    table = f"stp/{det}"

    evtids_full = lh5.read_as(f"{table}/evtid", stp_file, "np")

    chunks = list(_iter_event_aligned_chunks(stp_file, table, buffer))
    assert len(chunks) > 0

    # concatenated chunks must reproduce the full evtid column exactly
    evtids_concat = np.concatenate([c.view_as("ak").evtid.to_numpy() for c in chunks])
    assert np.array_equal(evtids_concat, evtids_full)

    # every chunk must end on an event boundary: no evtid is split across chunks
    for c in chunks[:-1]:
        assert len(c) > 0
    seen_last = None
    for c in chunks:
        ev = c.view_as("ak").evtid.to_numpy()
        if seen_last is not None:
            assert ev[0] != seen_last
        seen_last = ev[-1]


def test_iter_event_aligned_chunks_empty(tmptestdir):
    f = tmptestdir / "empty.lh5"
    lh5.write(
        Table(ak.Array({"evtid": np.array([], dtype=np.int64)})),
        "stp/det1",
        f,
        wo_mode="of",
    )
    assert list(_iter_event_aligned_chunks(str(f), "stp/det1", 1000)) == []


def test_iter_event_aligned_chunks_buffer_smaller_than_event(tmptestdir):
    # one event with many steps; buffer is smaller than the event size
    f = tmptestdir / "big_event.lh5"
    lh5.write(
        Table(ak.Array({"evtid": np.zeros(500, dtype=np.int64)})),
        "stp/det1",
        f,
        wo_mode="of",
    )
    chunks = list(_iter_event_aligned_chunks(str(f), "stp/det1", 10))
    assert len(chunks) == 1
    assert len(chunks[0]) == 500
