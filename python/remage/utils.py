from __future__ import annotations

import logging
import sys

import colorlog


def _setup_log() -> logging.Logger:
    """Setup a colored logger for this package."""

    logger = logging.getLogger("remage")

    if sys.stderr.isatty():
        fmt = "%(log_color)s[%(levelname)-7s ->%(reset)s %(message)s"

        handler = colorlog.StreamHandler()
        handler.setFormatter(colorlog.ColoredFormatter(fmt))
        logger.addHandler(handler)

    logger.setLevel(logging.INFO)

    return logger
