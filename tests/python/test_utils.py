# Copyright (C) 2026 The remage developers
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <https://www.gnu.org/licenses/>.

"""Unit tests for pure helpers in :mod:`remage.utils`."""

from __future__ import annotations

import pytest
from remage.utils import _to_list, sanitize_macro_cmds


def test_to_list_wraps_scalar():
    assert _to_list("a") == ["a"]
    assert _to_list(42) == [42]


def test_to_list_preserves_sequence():
    assert _to_list(["a", "b"]) == ["a", "b"]
    assert _to_list(("a", "b")) == ("a", "b")


def test_sanitize_macro_cmds_strips_blank_and_comment_lines():
    text = """
# leading comment
/run/initialize

  /RMG/Generator/Select GPS
# mid comment
/run/beamOn 1
"""
    assert sanitize_macro_cmds(text) == [
        "/run/initialize",
        "/RMG/Generator/Select GPS",
        "/run/beamOn 1",
    ]


def test_sanitize_macro_cmds_accepts_list_and_iterable():
    cmds = [
        "  /run/initialize  ",
        "# ignore",
        "",
        "/run/beamOn 10",
    ]
    assert sanitize_macro_cmds(cmds) == ["/run/initialize", "/run/beamOn 10"]
    assert sanitize_macro_cmds(iter(["/a", "#b", "/c"])) == ["/a", "/c"]


def test_sanitize_macro_cmds_splits_embedded_newlines_in_list_items():
    cmds = ["/run/initialize\n/run/beamOn 1", "# comment\n/RMG/Generator/Select GPS"]
    assert sanitize_macro_cmds(cmds) == [
        "/run/initialize",
        "/run/beamOn 1",
        "/RMG/Generator/Select GPS",
    ]


def test_sanitize_macro_cmds_rejects_non_string_items():
    with pytest.raises(TypeError, match="macro command must be a string"):
        sanitize_macro_cmds(["/run/initialize", 123])
