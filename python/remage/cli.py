from __future__ import annotations

import contextlib
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
from .ipc import IpcResult, ipc_thread_fn

log = logging.getLogger(__name__)


def _run_remage_cpp(
    args: list[str] | None = None,
) -> tuple[int, signal.Signals, IpcResult]:
    """run the remage-cpp executable and return the exit code as seen in bash."""

    # open pipe for IPC C++ -> python.
    pipe_r, pipe_w = os.pipe()
    os.set_inheritable(pipe_r, False)
    os.set_inheritable(pipe_w, True)

    if args is None:
        # reuse our own argv[0] to have helpful help messages.
        # but is is expanded (by the kernel?), so find out if we are in $PATH.
        argv = list(sys.argv)
        exe_name = Path(argv[0]).name
        if shutil.which(exe_name) == sys.argv[0]:
            argv[0] = exe_name
    else:
        argv = [REMAGE_CPP_EXE_PATH, *args]

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
    termsig = signal.Signals(-proc.returncode) if proc.returncode < 0 else None
    return ec, termsig, IpcResult(unhandled_ipc_messages)


def _setup_log() -> logging.Logger:
    """Setup a colored logger for this package."""

    logger = logging.getLogger("remage")

    if sys.stderr.isatty():
        fmt = "%(log_color)s[%(levelname)s %(name)s%(reset)s %(message)s"

        handler = colorlog.StreamHandler()
        handler.setFormatter(colorlog.ColoredFormatter(fmt))
        logger.addHandler(handler)

    logger.setLevel(logging.DEBUG)

    return logger


def _cleanup_tmp_files(ipc_info: IpcResult) -> None:
    """Remove temporary files created by the C++ application, that might not have been cleaned up."""
    tmp_files = ipc_info.get("tmpfile")
    for tmp_file in tmp_files:
        p = Path(tmp_file)
        with contextlib.suppress(Exception):
            p.unlink(missing_ok=True)


def remage_run(
    args: list[str] | None = None,
    *,
    raise_on_error: bool = True,
    raise_on_warning: bool = False,
) -> tuple[int, IpcResult]:
    """
    Run the remage simulation utility with the provided args.

    Notes
    -----
    This is the main entry point for users wanting to run remage from python code.

    Parameters
    ----------
    args
        argument list, as passed to the remage CLI utility.
    raise_on_error
        raise a :class:`RuntimeError` when an error in the C++ application occurs. This
        applies to non-fatal errors being logged as well as fatal errors. If false, the
        function only returns the error code, the python-based post-processing will be
        skipped in any case.
    raise_on_warning
        raise a :class:`RuntimeError` when a warning (or error) is logged in the C++
        application. If false, warnings are only logged and the python-based
        post-processing will be run normally.
    """
    logger = _setup_log()

    ec, termsig, ipc_info = _run_remage_cpp(args)
    # print an error message for the termination signal, similar to what bash does.
    if termsig not in (None, signal.SIGINT, signal.SIGPIPE):
        log.error(
            "remage-cpp exited with signal %s (%s)",
            termsig.name,
            signal.strsignal(termsig),
        )

    # clean-up should run always, irrespective of exit code.
    _cleanup_tmp_files(ipc_info)

    if ec not in [0, 2]:
        # remage had an error (::fatal -> ec==134 (SIGABRT); ::error -> ec==1)
        # ec==2 is just a warning, continue in the execution flow.
        if raise_on_error or raise_on_warning:
            msg = "error while running remage-cpp"
            raise RuntimeError(msg)
        return ec, ipc_info

    if ec == 2 and raise_on_warning:
        msg = "warning while running remage-cpp"
        raise RuntimeError(msg)

    assert termsig is None  # now we should only have had a graceful exit.

    # setup logging based on log level from C++.
    log_level = ipc_info.get_single("loglevel", "summary")
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
    _output_files = ipc_info.get("output")

    return ec, ipc_info


def remage_cli() -> int:
    return remage_run(raise_on_error=False)[0]
