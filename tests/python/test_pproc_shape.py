from __future__ import annotations

import awkward as ak
from lgdo import Array, Table
from reboost import units
from remage.reshaping import _group_by_time


def test_time_group():
    # time units are ns
    in_arr_evtid = Table(
        {
            "evtid": Array(
                [1, 1, 1, 2, 2, 2, 2, 2, 11, 12, 12, 12, 15, 15, 15, 15, 15]
            ),
            "time": units.attach_units(
                Array(
                    [
                        0,
                        -2000.0,
                        3000.0,
                        0.0,
                        100.0,
                        1200.0,
                        17000.0,
                        17010,
                        0,
                        0,
                        0,
                        -5000,
                        150,
                        151,
                        152,
                        3000,
                        3100,
                    ]
                ),
                "ns",
            ),
        }
    )

    in_tab = Table(in_arr_evtid)

    # 1us =1000ns
    in_ak = in_tab.view_as("ak", with_units=True)
    out = _group_by_time(in_ak, window_us=1)
    out_ak = out.view_as("ak", with_units=True)

    assert ak.all(
        out_ak.evtid
        == [
            [1],
            [1],
            [1],
            [2, 2],
            [2],
            [2, 2],
            [11],
            [12],
            [12, 12],
            [15, 15, 15],
            [15, 15],
        ]
    )
    assert ak.all(
        out_ak.time
        == [
            [-2000.0],
            [0.0],
            [3000.0],
            [0.0, 100.0],
            [1200.0],
            [17000.0, 17010.0],
            [0.0],
            [-5000.0],
            [0, 0],
            [150, 151, 152],
            [3000, 3100],
        ]
    )

    # test units
    assert units.get_unit_str(out_ak.time) == "ns"
