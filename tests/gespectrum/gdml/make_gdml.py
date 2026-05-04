#!/bin/python3

from __future__ import annotations

from dbetto import AttrsDict, utils
from pygeomhades import core, metadata
from pygeomtools import write_pygeom


def meta_dummy(self):
    self.hardware = AttrsDict(utils.load_dict("../dummy-metadata.yaml"))


metadata.PublicLegendMetadataProxy.__init__ = meta_dummy
metadata.PublicHadesMetadataProxy.__init__ = meta_dummy

config = AttrsDict(utils.load_dict("hades-test.yaml"))
registry = core.construct(
    config,
    public_geometry=True,  # use the patched .
)

# commit auxvals, and write to GDML file if requested.
write_pygeom(registry, "hades-test.gdml")
