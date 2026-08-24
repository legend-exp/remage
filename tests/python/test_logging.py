# Copyright (C) 2025 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

from __future__ import annotations

import io
import logging
import sys

import pytest
from remage import logging as rmg_logging


def test_console_print():
    logger = rmg_logging.setup_log()
    rmg_logging.set_logging_level(logger, "debug")

    # capture the logger output directly, independent of the default stderr
    # handler (which binds its stream at construction time).
    buffer = io.StringIO()
    handler = logging.StreamHandler(buffer)
    handler.setLevel(logging.DEBUG)
    logger.addHandler(handler)
    try:
        logger.debug("This is a debug message")
        logger.detail("This is a detail message")
        logger.info("This is an info message")
        logger.warning("This is a warning message")
        logger.error("This is an error message")
        logger.critical("This is a critical message")
    finally:
        logger.removeHandler(handler)

    output = buffer.getvalue()
    for message in (
        "This is a debug message",
        "This is a detail message",
        "This is an info message",
        "This is a warning message",
        "This is an error message",
        "This is a critical message",
    ):
        assert message in output


def test_set_logging_level_maps_rmg_names():
    logger = rmg_logging.setup_log()
    # case-insensitive via capitalize()
    rmg_logging.set_logging_level(logger, "warning")
    assert logger.level == logging.WARNING
    rmg_logging.set_logging_level(logger, "Summary")
    assert logger.level == logging.INFO
    rmg_logging.set_logging_level(logger, "detail")
    assert logger.level == rmg_logging.DETAIL


def test_set_logging_level_unknown_raises():
    logger = rmg_logging.setup_log()
    with pytest.raises(KeyError):
        rmg_logging.set_logging_level(logger, "nope")


def test_setup_log_idempotent_handlers():
    logger = rmg_logging.setup_log()
    n_before = sum(
        1 for h in logger.handlers if getattr(h, "_remage_default_handler", False)
    )
    rmg_logging.setup_log()
    n_after = sum(
        1 for h in logger.handlers if getattr(h, "_remage_default_handler", False)
    )
    assert n_before == 1
    assert n_after == 1


def test_supports_color_false_without_tty(monkeypatch):
    monkeypatch.setattr(sys.stderr, "isatty", lambda: False)
    monkeypatch.setenv("TERM", "xterm")
    assert rmg_logging.supports_color() is False


def test_supports_color_true_with_tty_and_term(monkeypatch):
    monkeypatch.setattr(sys.stderr, "isatty", lambda: True)
    monkeypatch.setenv("TERM", "xterm-256color")
    assert rmg_logging.supports_color() is True
