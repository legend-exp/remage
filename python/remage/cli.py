from __future__ import annotations

import logging
import os
import shutil
import signal
import subprocess
import sys
import threading
from pathlib import Path

import colorlog

from .cpp_config import REMAGE_CPP_EXE_PATH
from .ipc import ipc_thread_fn

log = logging.getLogger(__name__)


def _run_remage_cpp() -> int:
    """run the remage-cpp executable and return the exit code as seen in bash."""

    # open pipe for IPC C++ -> python.
    pipe_r, pipe_w = os.pipe()
    os.set_inheritable(pipe_r, False)
    os.set_inheritable(pipe_w, True)

    # reuse our own argv[0] to have helpful help messages.
    # but is is expanded (by the kernel?), so find out if we are in $PATH.
    argv = list(sys.argv)
    exe_name = Path(argv[0]).name
    if shutil.which(exe_name) == sys.argv[0]:
        argv[0] = exe_name

    if "--pipe-fd" in argv[1:]:
        msg = "cannot pass --pipe-fd"
        raise RuntimeError(msg)

    proc = subprocess.Popen(
        [argv[0], "--pipe-fd", str(pipe_w), *argv[1:]],
        executable=REMAGE_CPP_EXE_PATH,
        pass_fds=(pipe_w,),
    )

    # close _our_ writing end of the pipe.
    os.close(pipe_w)

    # propagate signals to the C++ executable.
    def new_signal_handler(sig: int, _):
        proc.send_signal(sig)

    signals = [
        signal.SIGHUP,
        signal.SIGINT,
        signal.SIGQUIT,
        signal.SIGTERM,
        signal.SIGTSTP,  # SIGSTOP cannot be caught, and will do nothing...
        signal.SIGCONT,
        signal.SIGUSR1,
        # signal.SIGUSR2 is for internal IPC communication.
        signal.SIGWINCH,
    ]

    old_signal_handlers = [signal.signal(sig, new_signal_handler) for sig in signals]

    # start a thread listening for IPC messages.
    # remage-cpp will only continue to do real work after we handled one sync message.
    unhandled_ipc_messages = []
    ipc_thread = threading.Thread(
        target=ipc_thread_fn, args=(pipe_r, proc, unhandled_ipc_messages)
    )
    ipc_thread.start()

    # wait for C++ executable to finish.
    proc.wait()

    # restore signal handlers again, before running more python code.
    for sig, handler in zip(signals, old_signal_handlers):
        signal.signal(sig, handler)

    # close the IPC pipe and end IPC handling.
    ipc_thread.join()

    ec = 128 - proc.returncode if proc.returncode < 0 else proc.returncode
    return ec, unhandled_ipc_messages


def _setup_log() -> None:
    """Setup a colored logger for this package."""

    logger = logging.getLogger("remage")

    if sys.stderr.isatty():
        fmt = "%(log_color)s[%(levelname)s %(name)s%(reset)s %(message)s"

        handler = colorlog.StreamHandler()
        handler.setFormatter(colorlog.ColoredFormatter(fmt))
        logger.addHandler(handler)

    logger.setLevel(logging.DEBUG)

    return logger


def remage_cli() -> None:
    logger = _setup_log()

    ec, ipc_info = _run_remage_cpp()
    if ec not in [0, 2]:
        # remage had an error (::fatal -> ec==134 (SIGABRT); ::error -> ec==1)
        # ec==2 is just a warning, continue in the execution flow.
        sys.exit(ec)

    # setup logging based on log level from C++.
    log_level = next(
        (msg[1] for msg in ipc_info if len(msg) == 2 and msg[0] == "loglevel"),
        "summary",
    )
    levels_rmg_to_py = {
        "debug": logging.DEBUG,
        "detail": logging.INFO,
        "summary": logging.INFO,
        "warning": logging.WARNING,
        "error": logging.ERROR,
        "fatal": logging.CRITICAL,
        "nothing": logging.CRITICAL,
    }
    logger.setLevel(levels_rmg_to_py[log_level])

    # TODO: further post-processing
    output_files = [msg[1] for msg in ipc_info if len(msg) == 2 and msg[0] == "output"]
    print("output files to merge:", *output_files)  # noqa: T201

    sys.exit(ec)
