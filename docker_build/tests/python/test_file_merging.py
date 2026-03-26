from __future__ import annotations

import re
from pathlib import Path

from remage import remage_run


def get_hidden_lh5(directory=".") -> list[Path]:
    return [
        p
        for p in Path(directory).iterdir()
        if p.is_file() and re.match(r"\..*\.lh5$", p.name)
    ]


def check_exists_and_remove(file):
    f = Path(file)

    if f.is_file():
        f.unlink()
        return True

    return False


def test_file_merging():
    output = Path("output.lh5")

    # remove all lh5 files (precaution)
    for p in Path().iterdir():
        if p.is_file() and re.match(r".*\.lh5$", p.name):
            p.unlink()

    remage_run(
        "macros/run.mac",
        gdml_files="gdml/geometry.gdml",
        output=output,
    )

    assert get_hidden_lh5() == []
    assert check_exists_and_remove(output)

    remage_run(
        "macros/run.mac",
        gdml_files="gdml/geometry.gdml",
        output=output,
        threads=2,
    )

    assert get_hidden_lh5() == []
    assert not check_exists_and_remove(output)

    for t in [0, 1]:
        assert check_exists_and_remove(f"{output.stem}_t{t}.lh5")

    remage_run(
        "macros/run.mac",
        gdml_files="gdml/geometry.gdml",
        output=output,
        threads=2,
        merge_output_files=True,
    )

    assert get_hidden_lh5() == []
    assert check_exists_and_remove(output)

    for t in [0, 1]:
        assert not check_exists_and_remove(f"{output.stem}_t{t}.lh5")
