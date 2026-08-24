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

"""Unit tests for IPC message parsing and :class:`remage.ipc.IpcResult`."""

from __future__ import annotations

import pytest
from remage.ipc import IpcResult, handle_ipc_message


def _msg(*records: str, blocking: bool = False) -> str:
    """Build a wire-format IPC message (records joined by RS, trailing GS)."""
    body = "\x1e".join(records)
    if blocking:
        body += "\x05"
    return body + "\x1d"


def test_handle_ipc_message_non_blocking_simple():
    is_blocking, parsed, is_fatal, proc_num = handle_ipc_message(
        _msg("0", "output", "out.lh5"), []
    )
    assert is_blocking is False
    assert is_fatal is False
    assert proc_num == 0
    assert parsed == ["output", "out.lh5"]


def test_handle_ipc_message_unit_separator_tuples():
    # value record contains US-separated units -> tuple on python side
    body = "1\x1ekey\x1eval0\x1fval1\x1d"
    is_blocking, parsed, is_fatal, proc_num = handle_ipc_message(body, [])
    assert is_blocking is False
    assert is_fatal is False
    assert proc_num == 1
    assert parsed == ["key", ("val0", "val1")]


def test_handle_ipc_message_blocking_unknown_key_returned():
    is_blocking, parsed, is_fatal, proc_num = handle_ipc_message(
        _msg("0", "custom_block", "x", blocking=True), []
    )
    assert is_blocking is True
    assert is_fatal is False
    assert proc_num == 0
    assert parsed == ["custom_block", "x"]


def test_handle_ipc_message_blocking_ipc_available_consumed(monkeypatch):
    import remage.ipc as ipc_mod

    monkeypatch.setattr(ipc_mod, "__version__", "9.9.9")
    is_blocking, parsed, is_fatal, proc_num = handle_ipc_message(
        _msg("0", "ipc_available", "9.9.9", blocking=True), []
    )
    assert is_blocking is True
    assert parsed is None
    assert is_fatal is False
    assert proc_num == 0


def test_handle_ipc_message_blocking_ipc_available_version_mismatch(monkeypatch):
    import remage.ipc as ipc_mod

    monkeypatch.setattr(ipc_mod, "__version__", "1.0.0")
    monkeypatch.delenv("REMAGE_CPP_PATH", raising=False)
    is_blocking, parsed, is_fatal, _ = handle_ipc_message(
        _msg("0", "ipc_available", "2.0.0", blocking=True), []
    )
    assert is_blocking is True
    assert parsed is None
    assert is_fatal is True


def test_handle_ipc_message_malformed_too_few_records():
    with pytest.raises(ValueError, match="too few records"):
        handle_ipc_message("0\x1d", [])


def test_handle_ipc_message_malformed_non_integer_proc():
    with pytest.raises(ValueError, match="not an integer"):
        handle_ipc_message(_msg("abc", "key", "val"), [])


def test_ipc_result_get_and_get_single():
    result = IpcResult(
        [
            ["output", "a.lh5"],
            ["output", "b.lh5"],
            ["ntuple", "det", "hits"],
            ["seed", "42"],
        ]
    )
    assert result.get("output") == ["a.lh5", "b.lh5"]
    assert result.get("ntuple", expected_len=2) == [["det", "hits"]]
    assert result.get_single("seed") == "42"
    assert result.get_single("missing", default="x") == "x"


def test_ipc_result_get_single_rejects_duplicates():
    result = IpcResult([["seed", "1"], ["seed", "2"]])
    with pytest.raises(RuntimeError, match="more than once"):
        result.get_single("seed")


def test_ipc_result_get_as_dict_set_remove():
    result = IpcResult(
        [
            ["meta", "k1", "v1"],
            ["meta", "k1", "v2"],
            ["meta", "k2", "v3"],
            ["other", "x"],
        ]
    )
    assert result.get_as_dict("meta") == {"k1": ["v1", "v2"], "k2": ["v3"]}

    result.set("other", ["y", "z"])
    assert result.get("other") == ["y", "z"]
    result.remove("other")
    assert result.get("other") == []
