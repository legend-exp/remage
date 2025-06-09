from __future__ import annotations

import logging
import os
import shutil
from pathlib import Path

log = logging.getLogger(__name__)


def _find_remage_from_config() -> tuple[Path, str] | None:
    try:
        from .cpp_config import REMAGE_CPP_EXE_PATH

        return Path(REMAGE_CPP_EXE_PATH), "config"
    except ImportError:
        return None


def find_remage_cpp() -> Path:
    """Find the remage executable, either by using the config stored into the package
    at build-time or by using the system PATH."""

    path = _find_remage_from_config()

    if path is None:
        path = shutil.which("remage-cpp")
        path = (Path(path), "path") if path is not None else None

    if path is None or not path[0].exists():
        missing = path[0] if path is not None else "None"
        msg = (
            f"remage-cpp executable '{missing}' not found. Ensure to use the right python "
            + "installation and/or set-up your PATH correctly."
        )
        raise RuntimeError(msg)

    assert isinstance(path[0], Path)
    # this is just for testing purposes, so that we can test on CI that our package is
    # built correctly and uses the right discovery mode.
    assert_origin = os.getenv("REMAGE_ASSERT_CPP_ORIGIN", "")
    assert assert_origin == "" or path[1] == assert_origin

    return path[0]
