# Copyright (C) 2025 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

import logging
import os
import sys

import colorlog
from rich.traceback import install as install_rich_traceback

_TRACEBACK_INSTALLED = False
# excepthook that was in place immediately before we installed rich's, so we can
# restore exactly that (and not clobber a hook the host app installed later).
_SAVED_EXCEPTHOOK: object = None
DETAIL = 11


LEVELS_RMG_TO_PY = {
    "Debug_event": logging.DEBUG,
    "Debug": logging.DEBUG,
    "Detail": DETAIL,
    "Summary": logging.INFO,
    "Warning": logging.WARNING,
    "Error": logging.ERROR,
    "Fatal": logging.CRITICAL,
    "Nothing": logging.CRITICAL + 1,
}

LEVEL_COLORS = {
    "Debug_event": "purple",
    "Debug": "purple",
    "Detail": "blue",
    "Summary": "green",
    "Warning": "yellow",
    "Error": "red",
    "Fatal": "bold_red",
}


def setup_log() -> logging.Logger:
    """Setup a colored logger for this package."""

    # register remage logging levels
    for k, v in LEVELS_RMG_TO_PY.items():
        logging.addLevelName(v, k)

    logger = logging.getLogger("remage")
    logger.setLevel(logging.DEBUG)
    logger.propagate = False

    # add a .detail(...) helper to the remage logger instance only, rather than
    # monkey-patching the stdlib logging.Logger class for the whole process.
    if not hasattr(logger, "detail"):
        logger.detail = lambda msg, *args, **kwargs: logger.log(  # type: ignore[attr-defined]
            DETAIL, msg, *args, **kwargs
        )

    # Avoid adding duplicate handlers if setup_log() is called multiple times.
    if not any(getattr(h, "_remage_default_handler", False) for h in logger.handlers):
        # Keep the visible formatting identical between colored and non-colored output.
        plain_fmt = "[%(levelname)-7s -> %(message)s"
        colored_fmt = "%(log_color)s[%(levelname)-7s ->%(reset)s %(message)s"

        if supports_color():
            handler = colorlog.StreamHandler()
            handler.setFormatter(
                colorlog.ColoredFormatter(colored_fmt, log_colors=LEVEL_COLORS)
            )
        else:
            handler = logging.StreamHandler()
            handler.setFormatter(logging.Formatter(plain_fmt))

        handler.setLevel(logging.DEBUG)
        handler._remage_default_handler = True  # type: ignore[attr-defined]
        logger.addHandler(handler)

    set_logging_level(logger, "Summary")

    return logger


def supports_color() -> bool:
    term = os.environ.get("TERM", None)
    terms = [
        "ansi",
        "color",
        "console",
        "cygwin",
        "gnome",
        "konsole",
        "kterm",
        "linux",
        "msys",
        "putty",
        "rxvt",
        "screen",
        "vt100",
        "xterm",
    ]
    return sys.stderr.isatty() and term is not None and any(t in term for t in terms)


def set_logging_level(logger, rmg_log_level):
    global _TRACEBACK_INSTALLED, _SAVED_EXCEPTHOOK  # noqa: PLW0603

    log_level = LEVELS_RMG_TO_PY[rmg_log_level.capitalize()]
    logger.setLevel(log_level)

    if log_level <= logging.DEBUG and not _TRACEBACK_INSTALLED:
        # snapshot the current hook right before overriding it, so we restore
        # whatever was active (possibly a host app's hook) rather than a stale
        # snapshot taken at import time.
        _SAVED_EXCEPTHOOK = sys.excepthook
        install_rich_traceback(show_locals=True, suppress=[logging])
        _TRACEBACK_INSTALLED = True

    # if back above DEBUG → restore the hook we replaced
    if log_level > logging.DEBUG and _TRACEBACK_INSTALLED:
        if _SAVED_EXCEPTHOOK is not None:
            sys.excepthook = _SAVED_EXCEPTHOOK
        _SAVED_EXCEPTHOOK = None
        _TRACEBACK_INSTALLED = False
