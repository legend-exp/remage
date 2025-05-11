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
from collections.abc import Mapping, Sequence
from pathlib import Path

import colorlog

from .find_remage import find_remage_cpp
from .ipc import IpcResult, ipc_thread_fn
from .post_proc import post_proc


def _run_remage_cpp(
    args: Sequence[str] | None = None,
    is_cli: bool = False,
) -> tuple[int, signal.Signals, IpcResult]:
    """run the remage-cpp executable and return the exit code as seen in bash."""

    remage_exe = find_remage_cpp()

    # open pipe for IPC C++ -> python.
    pipe_r, pipe_w = os.pipe()
    os.set_inheritable(pipe_r, False)
    os.set_inheritable(pipe_w, True)

    if args is None:
        argv = list(sys.argv)
        exe_name = Path(argv[0]).name
    else:
        argv = [remage_exe, *args]
        exe_name = Path(sys.argv[0]).name if is_cli else None

    if exe_name is not None and shutil.which(exe_name) == sys.argv[0]:
        # reuse our own argv[0] to have helpful help messages.
        # but is is expanded (by the kernel?), so find out if we are in $PATH.
        argv[0] = exe_name

    if any("--pipe-fd" in av for av in argv[1:]):
        msg = "cannot pass internal argument --pipe-fd"
        raise RuntimeError(msg)

    proc = subprocess.Popen(
        [argv[0], f"--pipe-fd={pipe_w}", *argv[1:]],
        executable=remage_exe,
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
        fmt = "%(log_color)s[%(levelname)-7s ->%(reset)s %(message)s"

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
    macros: Sequence[str] | str = (),
    *,
    gdml_files: Sequence[str] | str = (),
    output: str | None = None,
    threads: int = 1,
    overwrite_output: bool = False,
    merge_output_files: bool = False,
    flat_output: bool = False,
    time_window: float | None = None,
    macro_substitutions: Mapping[str, str] | None = None,
    log_level: str | None = None,
    raise_on_error: bool = True,
    raise_on_warning: bool = False,
) -> tuple[int, IpcResult]:
    """Run the remage simulation utility with the provided args.

    This is the main entry point for users wanting to run remage from Python
    code.

    Parameters
    ----------
    macros
        one or more remage/Geant4 macro command listings to execute.
    gdml_files
        supply one or more GDML files describing the experimental geometry.
    output
        output file for detector hits.
    threads
        set the number of threads used by remage.
    overwrite_output
        overwrite existing output files.
    merge_output_files
        merge output files created by individual remage threads.
    flat_output
        if ``False``, perform a reshaping of the output files so that each row
        in the output table contains data about all steps in a physical
        interaction in a detector, based on the time-window. This results in
        each column being a :class:`~lgdo.types.vectorofvectors.VectorOfVectors`.
        If ``True``, the output table will be flat with each row holding
        information about a single Geant4 step.
    time_window
        time window to group together steps into hits, in microseconds.
    macro_substitutions
        key-value-pairs that will be substituted in macros as Geant4 aliases.
    log_level
        logging level. One of `debug`, `detail`, `summary`, `warning`, `error`,
        `fatal`, `nothing`.
    raise_on_error
        raise a :class:`RuntimeError` when an error in the C++ application occurs. This
        applies to non-fatal errors being logged as well as fatal errors. If false, the
        function only returns the error code, the Python-based post-processing will be
        skipped in any case.
    raise_on_warning
        raise a :class:`RuntimeError` when a warning (or error) is logged in the C++
        application. If false, warnings are only logged and the python-based
        post-processing will be run normally.
    """
    args = []
    if not isinstance(gdml_files, str):
        for gdml in gdml_files:
            args.append(f"--gdml-files={gdml}")
    else:
        args.append(f"--gdml-files={gdml_files}")

    if output is not None:
        args.append(f"--output-file={output}")

    args.append(f"--threads={threads}")

    if merge_output_files:
        args.append("--merge-output-files")

    if flat_output:
        args.append("--flat-output")

    if time_window is not None:
        args.append(f"--time-window-in-us={time_window}")

    if overwrite_output:
        args.append("--overwrite")

    if macro_substitutions is not None:
        for subst_k, subst_v in macro_substitutions.items():
            args.append(f"--macro-substitutions={subst_k}={subst_v}")

    if log_level is not None:
        args.append(f"--log-level={log_level}")

    args.append("--")
    if not isinstance(macros, str):
        args.extend(macros)
    else:
        args.append(macros)

    return remage_run_from_args(
        args, raise_on_error=raise_on_error, raise_on_warning=raise_on_warning
    )


def remage_run_from_args(
    args: list[str] | None = None,
    *,
    raise_on_error: bool = True,
    raise_on_warning: bool = False,
) -> tuple[int, IpcResult]:
    """
    Run the remage simulation utility with the provided args.

    Parameters
    ----------
    args
        argument list, as passed to the remage CLI utility.
    raise_on_error
        see :meth:`remage_run`
    raise_on_warning
        see :meth:`remage_run`
    """
    logger = _setup_log()

    parser = argparse.ArgumentParser(allow_abbrev=False, add_help=False)

    # python only arguments
    parser.add_argument(
        "-m",
        "--merge-output-files",
        action="store_true",
        help="Merge output files from each remage thread into a single output file",
    )
    parser.add_argument(
        "--flat-output",
        action="store_true",
        help=(
            "Disable reshaping of the output table, such that each "
            "row refers to a single Geant4 step."
        ),
    )
    parser.add_argument(
        "--time-window-in-us",
        required=False,
        type=float,
        metavar="",
        default=10,
        help=(
            "Time window (in microseconds) used to reshape the output executable "
            "(see remage docs). Default is 10 microseconds."
        ),
    )
    py_args, cpp_args = parser.parse_known_args(args)

    ec, termsig, ipc_info = _run_remage_cpp(cpp_args, is_cli=args is None)

    # print an error message for the termination signal, similar to what bash does.
    if termsig not in (None, signal.SIGINT, signal.SIGPIPE):
        logger.error(
            "remage-cpp exited with signal %s (%s)",
            termsig.name,
            signal.strsignal(termsig),
        )

    # clean-up should run always, irrespective of exit code.
    _cleanup_tmp_files(ipc_info)

    if "-h" in cpp_args or "--help" in cpp_args:
        print("\n".join(parser.format_help().split("\n")[3:]))  # noqa: T201

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

    # apply python-based post-processing.
    post_proc(
        ipc_info,
        py_args.flat_output,
        py_args.merge_output_files,
        py_args.time_window_in_us,
    )

    return ec, ipc_info


def remage_cli() -> int:
    return remage_run_from_args(raise_on_error=False)[0]
