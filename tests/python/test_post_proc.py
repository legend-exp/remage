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
    import lgdo

    monkeypatch.setattr(lgdo.lh5, "ls", DummyLH5.ls)


def test_filters_known_detectors_and_appends_vtx():
    file_path = "dummy_file.lh5"
    detectors = ["det1", "det2"]
    result = post_proc.get_extra_tables(file_path, detectors)
    # Expect to include the extras in the order they appear, including duplicates, then '/vtx'
    assert result == ["stp/extra1", "stp/extra2", "stp/extra1", "vtx"]


def test_get_reboost_config():
    reshape = ["A", "B"]
    other = ["vtx"]
    config = post_proc.get_rebooost_config(reshape, other, time_window=5.5)
    expected = {
        "processing_groups": [
            {
                "name": "all",
                "detector_mapping": [{"output": "A"}, {"output": "B"}],
                "hit_table_layout": "reboost.shape.group.group_by_time(STEPS, 5.5)",
            },
        ],
        "forward": ["vtx"],
    }
    assert config == expected
