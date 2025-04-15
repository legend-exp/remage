from __future__ import annotations

import contextlib
import logging
import os
import shutil
import signal
import subprocess
import sys
import threading
from collections.abc import Iterable
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
    macros: Iterable[str] | str = (),
    *,
    gdmls: Iterable[str] | str = (),
    output: str | None = None,
    threads: int = 1,
    overwrite_output: bool = False,
    macro_substitutions: dict[str, str] | None = None,
    log_level: str | None = None,
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
    macros
        one or more remage/Geant4 macro command listings to execute.
    gdmls
        supply one or more GDML files describing the experimental geometry.
    output
        output file for detector hits.
    threads
        set the number of threads used by remage
    overwrite_output
        overwrite existing output files
    macro_substitutions
        key-value-pairs that will be substituted in macros as Geant4 aliases.
    log_level
        logging level. One of `debug`, `detail`, `summary`, `warning`, `error`.
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
    args = []
    if not isinstance(gdmls, str):
        for gdml in gdmls:
            args.append(f"--gdml-files={gdml}")
    else:
        args.append(f"--gdml-files={gdmls}")

    if output is not None:
        args.append(f"--output-file={output}")

    args.append(f"--threads={threads}")

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

    # output post-processing (merging multiple LH5 files)
    output_files = ipc_info.get("output")
    main_output_file = ipc_info.get_single("output_main", None)
    overwrite_output = ipc_info.get_single("overwrite_output", "0") == "1"  # noqa: F841
    # we might have no output file.
    if len(output_files) > 1 and main_output_file is not None:
        assert main_output_file not in output_files
        output_file_exts = {
            Path(p).suffix.lower() for p in [*output_files, main_output_file]
        }
        if output_file_exts == {".lh5"}:
            pass
            # lh5.concat.lh5concat(
            #     lh5_files=output_files,
            #     output=main_output_file,
            #     overwrite=overwrite_output,
            # )
            # set the merged output file for downstream consumers.
            # ipc_info.set("output", [main_output_file])
            # delete un-merged output files.
            # for f in output_files:
            #     Path(f).unlink()
    elif len(output_files) > 1:
        # no main output file, which is wrong.
        msg = "invalid output information returned over ipc"
        raise ValueError(msg)
    ipc_info.remove("output_main")

    return ec, ipc_info


def remage_cli() -> int:
    return remage_run_from_args(raise_on_error=False)[0]
