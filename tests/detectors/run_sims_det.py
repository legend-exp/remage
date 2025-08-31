from __future__ import annotations

from pathlib import Path

from remage import remage_run


def replace_lines(
    input_file: str, output_file: str, replacements: dict[str, str]
) -> None:
    """Replaces lines in a file that match given patterns."""
    with Path(input_file).open(encoding="utf-8") as f:
        lines = f.readlines()

    with Path(output_file).open("w", encoding="utf-8") as f:
        for line in lines:
            line_t = line
            for pattern, replacement in replacements.items():
                if pattern in line:
                    line_t = replacement + "\n"
                    break
            f.write(line_t)


def run_sim(run_name: str, register_dets_command: str):
    replacements = {
        "$REGISTER_DETS": register_dets_command,
    }

    replace_lines("macros/template.mac", "macros/run.mac", replacements)

    remage_run(
        "macros/run.mac",
        gdml_files="gdml/geometry.gdml",
        output=f"{run_name}.lh5",
        overwrite_output=True,
    )


# Define the different detector registration commands to test
runs = [
    ("det-from-gdml", "/RMG/Geometry/RegisterDetectorsFromGDML Germanium"),
    ("det-from-regex", "/RMG/Geometry/RegisterDetector Germanium germanium.* 101"),
    (
        "det-from-regex-copynr",
        "/RMG/Geometry/RegisterDetector Germanium germanium.* 101 .*",
    ),
    (
        "det-from-name",
        "/RMG/Geometry/RegisterDetector Germanium germanium_det1 101\n/RMG/Geometry/RegisterDetector Germanium germanium_det2 102",
    ),
    (
        "det-from-both",
        "/RMG/Geometry/RegisterDetectorsFromGDML Germanium\n/RMG/Geometry/RegisterDetector Scintillator dif.* 104",
    ),
]

# Run each simulation
for run_name, register_command in runs:
    run_sim(run_name, register_command)
