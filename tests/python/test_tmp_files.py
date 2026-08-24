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

"""Unit tests for temporary file rename helpers in :mod:`remage.post_proc`."""

from __future__ import annotations

from pathlib import Path

import pytest
from remage.post_proc import make_tmp, tmp_renamed_files, un_make_tmp


def test_make_tmp_and_un_make_tmp_roundtrip(tmp_path: Path):
    f1 = tmp_path / "out.lh5"
    f2 = tmp_path / "other.lh5"
    f1.write_text("a", encoding="utf-8")
    f2.write_text("b", encoding="utf-8")

    hidden = make_tmp([str(f1), str(f2)])
    assert not f1.exists()
    assert not f2.exists()
    assert all(Path(h).name.startswith(".") for h in hidden)
    assert all(Path(h).exists() for h in hidden)

    restored = un_make_tmp(hidden)
    assert set(restored) == {str(f1), str(f2)}
    assert f1.read_text(encoding="utf-8") == "a"
    assert f2.read_text(encoding="utf-8") == "b"


def test_make_tmp_accepts_single_path(tmp_path: Path):
    f = tmp_path / "one.lh5"
    f.write_text("x", encoding="utf-8")
    hidden = make_tmp(str(f))
    assert len(hidden) == 1
    assert Path(hidden[0]).exists()
    un_make_tmp(hidden)
    assert f.exists()


def test_tmp_renamed_files_success_deletes_hidden(tmp_path: Path):
    f = tmp_path / "data.lh5"
    f.write_text("payload", encoding="utf-8")

    with tmp_renamed_files([str(f)]) as hidden:
        assert not f.exists()
        assert Path(hidden[0]).exists()

    # on success the hidden temp is unlinked and the original stays gone
    assert not f.exists()
    assert not Path(hidden[0]).exists()


def test_tmp_renamed_files_restores_on_error(tmp_path: Path):
    f = tmp_path / "data.lh5"
    f.write_text("payload", encoding="utf-8")

    with pytest.raises(RuntimeError, match="boom"), tmp_renamed_files([str(f)]):
        raise RuntimeError("boom")

    assert f.exists()
    assert f.read_text(encoding="utf-8") == "payload"
