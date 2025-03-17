from __future__ import annotations

import argparse
import contextlib
import logging
import os
import shutil
import signal
import subprocess
import sys
import threading
from collections.abc import Sequence
from pathlib import Path

import colorlog

from . import preproc
from .cpp_config import REMAGE_CPP_EXE_PATH
from .ipc import IpcResult, ipc_thread_fn

log = logging.getLogger(__name__)

LOG_LEVELS_RMG2PY = {
    "debug": logging.DEBUG,
    "detail": logging.INFO,
    "summary": logging.INFO,
    "warning": logging.WARNING,
    "error": logging.ERROR,
    "fatal": logging.CRITICAL,
    "nothing": logging.CRITICAL,
}


def _run_remage_cpp(
    args: Sequence[str] | None = None,
) -> tuple[int, signal.Signals, IpcResult]:
    """Run the remage-cpp executable and return its exit code."""

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


def _cleanup_cpp_tmp_files(ipc_info: IpcResult) -> None:
    """Remove temporary files created by the C++ application, that might not have been cleaned up."""
    tmp_files = ipc_info.get("tmpfile")
    for tmp_file in tmp_files:
        p = Path(tmp_file)
        with contextlib.suppress(Exception):
            p.unlink(missing_ok=True)


def _parse_remage_args(
    args: Sequence[str],
) -> tuple[argparse.Namespace, argparse.Namespace, list[str]]:
    # NOTE: duplicated in src/remage.cc
    # NOTE: no need to duplicate cpp-only arguments

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-q",
        "--quiet",
        action="store_true",
        default=False,
        help="""Print only warnings and errors (same as --log-level=warning)""",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        default=False,
        help="""Increase program verbosity to maximum (same as --log-level=debug)""",
    )
    parser.add_argument(
        "--version", action="store_true", help="""Print remage's version and exit"""
    )
    parser.add_argument(
        "--version-rich",
        action="store_true",
        help="""Print versions of remage and its dependencies and exit""",
    )
    parser.add_argument(
        "-l",
        "--log-level",
        choices=["debug", "detail", "summary", "warning", "error", "fatal", "nothing"],
        default="summary",
        help="""Set the logging level""",
    )
    parser.add_argument(
        "-i",
        "--interactive",
        action="store_true",
        help="""Open an interactive macro command prompt""",
    )
    parser.add_argument(
        "-t",
        "--threads",
        help="""Set the number of threads used by remage""",
        default=1,
    )
    parser.add_argument(
        "-g",
        "--gdml-files",
        nargs="+",
        help="""Supply one or more GDML files describing the experimental geometry""",
    )
    parser.add_argument("-o", "--output-file", help="""The output file name""")
    parser.add_argument(
        "-w",
        "--overwrite",
        action="store_true",
        help="""Overwrite existing output files""",
    )
    parser.add_argument(
        "macros",
        nargs="*",
        default=[],
        help="""One or more remage/Geant4 macro command listings to execute""",
    )

    parser.add_argument(
        "--substitute-in-macro",
        "-s",
        nargs="+",
        default=None,
        metavar="KEY=VAL",
        help="key-value pairs (e.g., a=1 b=2 c=3)",
    )

    # parse them
    return parser.parse_args(args)


def remage_run(
    args: Sequence[str] | None = None, *, raise_error: bool = True
) -> tuple[int, IpcResult]:
    """Execute remage.

    Parameters
    ----------
    args
        list of command line arguments. ``None`` is reserved for
        non-python-console usage (i.e. used by the remage wrapper executable).
    raise_error
        raise an exception if the remage executable returns a non-null exit
        code.
    """
    logger = _setup_log()

    # args=None can be passed to parser.parse_args() and it will use sys.argv,
    # but here we want it to be not None because we want to pass it over to
    # process_template_macros_in_args()
    if args is None:
        args = list(sys.argv[1:])

    # pre-processing
    # need to sniff the args sent to remage-cpp
    parsed_args = _parse_remage_args(args)

    # set log level of the python wrapper
    # NOTE: same priority/logic as in src/remage.cc
    log_level = parsed_args.log_level
    if parsed_args.verbose:
        log_level = "debug"
    if parsed_args.quiet:
        log_level = "warning"

    logger.setLevel(LOG_LEVELS_RMG2PY[log_level])

    # user requests variable substitutions in the macro
    tmp_macros = None
    if parsed_args.substitute_in_macro is not None and parsed_args.macros is not None:
        tmp_macros = preproc.process_template_macros_in_args(
            args, parsed_args.macros, parsed_args.substitute_in_macro
        )

    msg = f"post-processed remage args: {args}"
    log.debug(msg)

    # run remage-cpp
    ec, termsig, ipc_info = _run_remage_cpp(args)

    # print an error message for the termination signal, similar to what bash does.
    if termsig not in (None, signal.SIGINT, signal.SIGPIPE):
        log.error(
            "remage-cpp exited with signal %s (%s)",
            termsig.name,
            signal.strsignal(termsig),
        )

    # clean-up should run always, irrespective of exit code.
    _cleanup_cpp_tmp_files(ipc_info)

    if ec not in [0, 2]:
        # remage had an error (::fatal -> ec==134 (SIGABRT); ::error -> ec==1)
        # ec==2 is just a warning, continue in the execution flow.
        if raise_error:
            msg = "error while running remage-cpp"
            raise RuntimeError(msg)

        return ec, ipc_info

    if tmp_macros is not None:
        [Path(f).unlink() for f in tmp_macros]

    assert termsig is None  # now we should only have had a graceful exit.

    # reset log level based on log level from C++ (in case it was modified via macro)
    log_level = ipc_info.get_single("loglevel", "summary")
    logger.setLevel(LOG_LEVELS_RMG2PY[log_level])

    # TODO: further post-processing
    # _output_files = ipc_info.get("output")

    return ec, ipc_info


def remage_cli() -> int:
    return remage_run(raise_error=False)[0]
