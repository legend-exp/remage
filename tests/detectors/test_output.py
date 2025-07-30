from __future__ import annotations

import difflib
import re
import subprocess
from pathlib import Path


def strip_ansi(text: str) -> str:
    return re.sub(r"\x1B\[[0-9;]*[mK]", "", text)


output_files = [
    "det-from-gdml.lh5",
    "det-from-regex.lh5",
    "det-from-name.lh5",
    "det-from-both.lh5",
]

dump_files = [
    "dumps/from-gdml.lh5.ls",
    "dumps/from-regex.lh5.ls",
    "dumps/from-name.lh5.ls",
    "dumps/from-both.lh5.ls",
]

for output_file, dump_file in zip(output_files, dump_files):
    result = subprocess.run(
        ["lh5ls", str(output_file)], capture_output=True, text=True, check=True
    )
    actual_output = strip_ansi(result.stdout).strip()
    expected_output = Path(dump_file).read_text().strip()
    if actual_output != expected_output:
        diff = "\n".join(
            difflib.unified_diff(
                expected_output.splitlines(),
                actual_output.splitlines(),
                fromfile=dump_file,
                tofile=output_file,
                lineterm="",
            )
        )
        msg = f"Mismatch in {output_file}:\n{diff}"
        raise AssertionError(msg)
