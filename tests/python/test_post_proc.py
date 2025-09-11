from __future__ import annotations

import pytest
from remage import post_proc


class DummyLH5:
    @staticmethod
    def ls(file, lh5_group=None):  # noqa: ARG004
        # Simulate various table listings
        return [
            "stp/det1",  # known detector
            "stp/extra1",
            "stp/det2",  # known detector
            "stp/extra2",
            "stp/extra1",  # duplicate extra to test preservation of duplicates
        ]


@pytest.fixture(autouse=True)
def patch_lh5(monkeypatch):
    # Monkey-patch lh5.ls
    import lh5

    monkeypatch.setattr(lh5, "ls", DummyLH5.ls)


def test_get_reboost_config():
    # the inputs are mappings <output scheme> -> [<table>, ...]. the same table name
    # can appear several times (one entry per detector and per worker thread/process),
    # and must be deduplicated in the resulting config.
    detector_info = {
        "RMGGermaniumOutputScheme": ["A", "A"],
        "RMGScintillatorOutputScheme": ["B"],
    }
    detector_info_aux = {"RMGGermaniumOutputScheme": ["vtx", "vtx"]}
    config = post_proc.get_reboost_config(
        detector_info, detector_info_aux, time_window=5.5
    )
    expected = {
        "processing_groups": [
            {
                "name": "default",
                "detector_mapping": [{"output": "A"}, {"output": "B"}],
                "hit_table_layout": "reboost.shape.group.group_by_time(STEPS, 5.5)",
                "operations": {
                    "evtid": "ak.fill_none(ak.firsts(HITS.evtid, axis=-1), 0)",
                    "t0": {
                        "expression": "ak.fill_none(ak.firsts(HITS.time, axis=-1), 0)",
                        "units": "ns",
                    },
                },
            },
        ],
        "forward": ["vtx"],
    }
    assert config == expected


def test_get_reboost_config_calorimeter():
    # calorimeter tables get their own processing group, excluded from the default one.
    detector_info = {
        "RMGGermaniumOutputScheme": ["A"],
        "RMGCalorimeterOutputScheme": ["calo", "calo"],
    }
    config = post_proc.get_reboost_config(detector_info, {}, time_window=10)

    groups = config["processing_groups"]
    assert len(groups) == 2

    # the default group must not contain the calorimeter tables
    assert groups[0]["name"] == "default"
    assert groups[0]["detector_mapping"] == [{"output": "A"}]

    # the calorimeter table is already in final, flat form (one row per detector
    # per event), so its group must not reshape it: no `hit_table_layout`, and
    # only a rename of the time column to `t0` for the coincidence map. only the
    # (deduplicated) calorimeter tables are processed.
    calo = groups[1]
    assert calo["name"] == "RMGCalorimeterOutputScheme"
    assert calo["detector_mapping"] == [{"output": "calo"}]
    assert "hit_table_layout" not in calo
    assert calo["operations"] == {"t0": {"expression": "HITS.time", "units": "ns"}}
    assert calo["outputs"] == ["evtid", "det_uid", "edep", "t0"]

    assert config["forward"] == []
