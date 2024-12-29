from __future__ import annotations

import subprocess
import sys

from .cpp_config import REMAGE_CPP_EXE_PATH


def remage_cli():
    result = subprocess.run([REMAGE_CPP_EXE_PATH] + sys.argv[1:], check=False)
    sys.exit(result.returncode)
