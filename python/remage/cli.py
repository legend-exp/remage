from __future__ import annotations

import argparse
import subprocess
import sys

from .cpp_config import REMAGE_CPP_EXE_PATH


def remage_cli():
    parser = argparse.ArgumentParser(
        prog="remage", description="remage's command-line interface"
    )

    parser.add_argument(
        "--version", action="store_true", help="""Print remage version and exit"""
    )

    args = parser.parse_args()

    cmdline = [REMAGE_CPP_EXE_PATH]
    if args.version:
        cmdline += ["--version"]

    result = subprocess.run(cmdline, check=False)

    sys.exit(result.returncode)
