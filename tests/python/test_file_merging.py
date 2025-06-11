from __future__ import annotations

from pathlib import Path

from remage import remage_run_from_args


def test_file_merging():
    output = Path("output.lh5")

    args = [
        "--gdml-files",
        "gdml/geometry.gdml",
        "--output-file",
        str(output),
        "--overwrite",
        "--quiet",
        "macros/run.mac",
    ]

    remage_run_from_args(args, raise_on_error=True)

    assert output.unlink() is None

    remage_run_from_args(["--threads", "2", *args], raise_on_error=True)

    assert not output.exists()

    for t in [0, 1]:
        assert Path(f"{output.stem}_t{t}.lh5").unlink() is None

    remage_run_from_args(
        ["--threads", "2", "--merge-output-files", *args], raise_on_error=True
    )

    assert output.unlink() is None

    for t in [0, 1]:
        assert not Path(f"{output.stem}_t{t}.lh5").exists()
