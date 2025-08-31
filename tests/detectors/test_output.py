from __future__ import annotations

from lgdo import lh5

output_files = [
    ("gdml", "det-from-gdml.lh5"),
    ("regex", "det-from-regex.lh5"),
    ("regex-copynr", "det-from-regex-copynr.lh5"),
    ("name", "det-from-name.lh5"),
]

# Check if the output rows exist
for registering_type, output_file in output_files:
    try:
        lh5.read_as("/stp/germanium_det1", output_file, "ak")
        lh5.read_as("/stp/germanium_det2", output_file, "ak")
    except Exception as err:
        msg = (
            f"Expected output rows for '{output_file}' do not exist. "
            f"Detectors not registered correctly from {registering_type}."
        )
        raise AssertionError(msg) from err

try:
    lh5.read_as("/stp/germanium_det1", "det-from-both.lh5", "ak")
    lh5.read_as("/stp/germanium_det2", "det-from-both.lh5", "ak")
    lh5.read_as("/stp/different_det", "det-from-both.lh5", "ak")
except Exception as err:
    msg = (
        "Expected output rows for 'det-from-both.lh5' do not exist. "
        "Detectors not registered correctly when using both functions."
    )
    raise AssertionError(msg) from err
