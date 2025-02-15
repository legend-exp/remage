from __future__ import annotations

import sys

import h5py

dump_lh5_datatype = len(sys.argv) > 2 and sys.argv[2] == "--dump-attrs"
hfile = h5py.File(sys.argv[1], "r")


def visit(name: str):
    if dump_lh5_datatype:
        print(name, ":", hfile[name].attrs.get("datatype"))
    else:
        print(name)


hfile.visit(visit)
