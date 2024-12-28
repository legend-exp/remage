from __future__ import annotations

import argparse
import subprocess
import sys

from .cpp_utils import REMAGE_CPP_EXE_PATH

# NOTE: when uv/pip installs the package and creates the executable for the cli,
# it hardcodes the path to the current python executable (e.g. the one of the
# virtualenv) in the script's shebang


def remage_cli():
    parser = argparse.ArgumentParser(
        prog="remage", description="remage's command-line interface"
    )

    # global options
    parser.add_argument(
        "--version", action="store_true", help="""Print remage version and exit"""
    )

    args = parser.parse_args()

    cmdline = [REMAGE_CPP_EXE_PATH]
    if args.version:
        cmdline += ["--version"]

    result = subprocess.run(cmdline, check=False)

    sys.exit(result.returncode)
