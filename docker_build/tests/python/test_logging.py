from __future__ import annotations

from remage import logging as rmg_logging


def test_console_print():
    logger = rmg_logging.setup_log()
    rmg_logging.set_logging_level(logger, "debug")

    logger.debug("This is a debug message")
    logger.detail("This is a detail message")
    logger.info("This is an info message")
    logger.warning("This is a warning message")
    logger.error("This is an error message")
    logger.critical("This is a critical message")
