from __future__ import annotations

import pytest
from remage import remage_run


def _remage_run(macros):
    remage_run(
        macros,
        gdml_files="gdml/geometry.gdml",
        output=None,
    )


def test_macro_input():
    _remage_run("macros/run.mac")
    _remage_run(["macros/run-a.mac", "macros/run-b.mac"])

    with pytest.raises(RuntimeError):
        _remage_run("this is nonsense")

    inline_cmds = ["/run/initialize", "/RMG/Generator/Select GPS", "/run/beamOn 1"]
    _remage_run(inline_cmds)
    for char in ("\n", ";", ","):
        _remage_run(char.join(inline_cmds))

    _remage_run(["macros/run-a.mac", "/RMG/Generator/Select GPS", "/run/beamOn 1"])
