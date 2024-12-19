#!/bin/env python3
from __future__ import annotations

import numpy as np
from lgdo import Array, Table, lh5

xloc = Array(np.array([0, 1, 2] * 10, dtype=np.float64), attrs={"units": "m"})
yloc = Array(np.array([3, 4, 5] * 10, dtype=np.float64), attrs={"units": "m"})
zloc = Array(np.array([6, 7, 8] * 10, dtype=np.float64), attrs={"units": "m"})

lh5.write(
    Table({"xloc": xloc, "yloc": yloc, "zloc": zloc}),
    "vtx/pos",
    lh5_file="vert.lh5",
    wo_mode="of",
)
