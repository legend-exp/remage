from __future__ import annotations

import sys

import h5py

dump_lh5_datatype = len(sys.argv) > 2 and sys.argv[2] == "--dump-attrs"


def visit(name: str):
    if dump_lh5_datatype:
        print(name, ":", dict(hfile[name].attrs))
    else:
        print(name)


with h5py.File(sys.argv[1], "r") as hfile:
    hfile.visit(visit)
