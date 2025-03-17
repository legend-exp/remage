from __future__ import annotations

from pathlib import Path

import pytest
from remage import preproc


def test_macro_preproc(tmpdir):
    tmp_macro = tmpdir / "macro_noproc.mac"

    macro_lines = [
        r"/RMG/Generator/PositionX ${XPOS}",
        r"/RMG/Generator/PositionY $YPOS",
        r"/RMG/Generator/PositionZ $$ZPOS",
        r"/RMG/Generator/Select ${GENERATOR}",
        r"/gps/particle e-",
        r"/gps/ang/type iso",
        r"/gps/energy $ENERGY keV",
    ]

    with tmp_macro.open("w") as f:
        f.writelines(line + "\n" for line in macro_lines)

    tmp_macro_preproc = preproc.process_template_macro(
        tmp_macro,
        {
            "XPOS": 1,
            "YPOS": 2,
            "ZPOS": 3,
            "ENERGY": 1111.11,
            "GENERATOR": "GPS",
        },
    )

    with tmp_macro_preproc.open() as f:
        proc_macro_lines = [line.rstrip("\n") for line in f]

    assert proc_macro_lines == [
        r"/RMG/Generator/PositionX 1",
        r"/RMG/Generator/PositionY 2",
        r"/RMG/Generator/PositionZ $ZPOS",
        r"/RMG/Generator/Select GPS",
        r"/gps/particle e-",
        r"/gps/ang/type iso",
        r"/gps/energy 1111.11 keV",
    ]


def test_bad_macro_template(tmpdir):
    tmp_macro = tmpdir / "macro_noproc.mac"

    macro_lines = [
        r"/RMG/Generator/PositionX ${XPOS}",
        r"/RMG/Generator/PositionY $YPOS",
    ]

    with tmp_macro.open("w") as f:
        f.writelines(line + "\n" for line in macro_lines)

    with pytest.raises(KeyError):
        preproc.process_template_macro(tmp_macro, {"XPOS": 1})

    macro_lines = [
        r"/RMG/Generator/PositionX $XPOS",
        r"/RMG/Generator/PositionY $ ",
    ]

    with tmp_macro.open("w") as f:
        f.writelines(line + "\n" for line in macro_lines)

    with pytest.raises(RuntimeError):
        preproc.process_template_macro(tmp_macro, {"XPOS": 1})


def test_macro_hijacking(tmpdir):
    tmp_macros = [f"{tmpdir}/macro{n}.mac" for n in range(3)]

    macro_lines = [
        r"/RMG/Generator/PositionX ${XPOS}",
        r"/RMG/Generator/PositionY $YPOS",
    ]

    for _file in tmp_macros:
        with Path(_file).open("w") as f:
            f.writelines(line + "\n" for line in macro_lines)

    args = ["-w", "--threads", "3", "--interactive", "--", *tmp_macros]

    preproc.process_template_macros_in_args(args, tmp_macros, ["XPOS=1", "YPOS=2"])

    assert args[: -len(tmp_macros)] == ["-w", "--threads", "3", "--interactive", "--"]
    for i, m in enumerate(tmp_macros):
        assert args[-len(tmp_macros) + i].endswith(Path(m).name)
