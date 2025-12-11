# Copyright (C) 2025 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

from __future__ import annotations

import os
import shutil
from pathlib import Path


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
