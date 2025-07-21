from __future__ import annotations

import logging
import os
import sys

import colorlog
from rich.traceback import install as install_rich_traceback

_ORIGINAL_EXCEPTHOOK = sys.excepthook
_TRACEBACK_INSTALLED = False
DETAIL = 11

# monkey-patch Logger to add the .detail(...) method
logging.Logger.detail = lambda self, msg, *args, **kwargs: self.log(
    DETAIL, msg, *args, **kwargs
)


LEVELS_RMG_TO_PY = {
    "Debug": logging.DEBUG,
    "Detail": DETAIL,
    "Summary": logging.INFO,
    "Warning": logging.WARNING,
    "Error": logging.ERROR,
    "Fatal": logging.CRITICAL,
    "Nothing": logging.CRITICAL + 1,
}

LEVEL_COLORS = {
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

    if supports_color():
        fmt = "%(log_color)s[%(levelname)-7s ->%(reset)s %(message)s"

        handler = colorlog.StreamHandler()
        handler.setFormatter(colorlog.ColoredFormatter(fmt, log_colors=LEVEL_COLORS))
        handler.setLevel(logging.DEBUG)
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
    return sys.stderr.isatty() and any(term in t for t in terms)


def set_logging_level(logger, rmg_log_level):
    global _TRACEBACK_INSTALLED  # noqa: PLW0603

    log_level = LEVELS_RMG_TO_PY[rmg_log_level.capitalize()]
    logger.setLevel(log_level)

    if log_level <= logging.DEBUG and not _TRACEBACK_INSTALLED:
        install_rich_traceback(show_locals=True, suppress=[logging])
        _TRACEBACK_INSTALLED = True

    # if back above DEBUG → restore original hook
    if log_level > logging.DEBUG and _TRACEBACK_INSTALLED:
        sys.excepthook = _ORIGINAL_EXCEPTHOOK
        _TRACEBACK_INSTALLED = False
