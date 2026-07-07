from __future__ import annotations

import io
import logging

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
