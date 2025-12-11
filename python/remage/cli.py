# Copyright (C) 2025 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
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

from . import logging as rmg_logging
from . import utils
from .find_remage import find_remage_cpp
from .ipc import IpcResult, ipc_thread_fn
from .post_proc import post_proc


def _run_remage_cpp(
    args: Sequence[str] | None = None,
    is_cli: bool = False,
    num_procs: int | None = 0,
) -> tuple[list[int], list[signal.Signals], IpcResult]:
    """run the remage-cpp executable and return the exit code as seen in bash."""
    logger = logging.getLogger("remage")

    remage_exe = find_remage_cpp()

    # open pipe for IPC C++ -> python.
    pipe_i_r, pipe_i_w = os.pipe()
    os.set_inheritable(pipe_i_r, False)
    os.set_inheritable(pipe_i_w, True)

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

    proc = []
    pipes_o = []
    num_procs = num_procs or 1
    for proc_num in range(num_procs):
        pipe_o_r, pipe_o_w = os.pipe()
        os.set_inheritable(pipe_o_r, True)
        os.set_inheritable(pipe_o_w, False)

        extra_args = [f"--pipe-o-fd={pipe_i_w}", f"--pipe-i-fd={pipe_o_r}"]
        if num_procs > 1:
            extra_args.append(f"--proc-num-offset={proc_num}")
        full_args = [str(argv[0]), *extra_args, *argv[1:]]
        msg = "Running command: " + " ".join(full_args)
        logger.debug(msg)

        proc.append(
            subprocess.Popen(
                full_args,
                executable=remage_exe,
                pass_fds=(pipe_i_w, pipe_o_r),
            )
        )
        pipes_o.append(pipe_o_w)

        os.close(pipe_o_r)  # close _our_ reading end of this pipe

    # close _our_ writing end of the pipe.
    os.close(pipe_i_w)

    # propagate signals to the C++ executable.
    def new_signal_handler(sig: int, _):
        for p in proc:
            p.send_signal(sig)

    signals = [
        signal.SIGHUP,
        signal.SIGINT,
        signal.SIGQUIT,
        signal.SIGTERM,
        signal.SIGTSTP,  # SIGSTOP cannot be caught, and will do nothing...
        signal.SIGCONT,
        signal.SIGUSR1,
        signal.SIGUSR2,
        signal.SIGWINCH,
    ]

    old_signal_handlers = [signal.signal(sig, new_signal_handler) for sig in signals]

    # start a thread listening for IPC messages.
    # remage-cpp will only continue to do real work after we handled one sync message.
    unhandled_ipc_messages = []
    ipc_thread = threading.Thread(
        target=ipc_thread_fn, args=(pipe_i_r, pipes_o, proc, unhandled_ipc_messages)
    )
    ipc_thread.start()

    # wait for C++ executable to finish.
    for p in proc:
        p.wait()

    # restore signal handlers again, before running more python code.
    for sig, handler in zip(signals, old_signal_handlers, strict=True):
        signal.signal(sig, handler)

    # close the IPC pipe and end IPC handling.
    ipc_thread.join()

    ec = [128 - p.returncode if p.returncode < 0 else p.returncode for p in proc]
    termsig = [
        signal.Signals(-p.returncode) if p.returncode < 0 else None for p in proc
    ]
    return ec, termsig, IpcResult(unhandled_ipc_messages)


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
    gdml_files: Sequence[str | Path] | str | Path = (),
    output: str | Path | None = None,
    threads: int = 1,
    procs: int = 1,
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
        set the number of threads used by remage. This cannot be combined with `procs`.
    procs
        set the number of processes used by remage. This cannot be combined with
        `threads`.
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
        logging level. One of `debug_event`, `debug`, `detail`, `summary`, `warning`,
        `error`, `fatal`, `nothing`.
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
    if not isinstance(gdml_files, str | Path):
        for gdml in gdml_files:
            args.append(f"--gdml-files={gdml}")
    else:
        args.append(f"--gdml-files={gdml_files}")

    if output is not None:
        args.append(f"--output-file={output!s}")

    if threads > 1 and procs > 1:
        msg = "Only one of threads or procs can be larger than one."
        raise ValueError(msg)
    args.append(f"--threads={threads}")
    args.append(f"--procs={procs}")

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

    args.extend(["--", *utils.sanitize_macro_cmds(macros)])

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
    logger = rmg_logging.setup_log()

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
    parser.add_argument(
        "--procs",
        "-P",
        type=int,
        help=(
            "Set the number of worker processes used by remage. Cannot be combined "
            "with --threads/-t."
        ),
    )
    parser.add_argument(
        "--ignore-warnings",
        action="store_true",
        help="Do not exit with exit code 2 when warnings occurred.",
    )
    py_args, cpp_args = parser.parse_known_args(args)

    ec, termsig, ipc_info = _run_remage_cpp(
        cpp_args, is_cli=args is None, num_procs=py_args.procs
    )
    ec = 1 if 1 in ec else max(ec)

    # print an error message for the termination signal, similar to what bash does.
    for proc_num, t in enumerate(termsig):
        if t not in (None, signal.SIGINT, signal.SIGPIPE):
            logger.error(
                "remage-cpp%s exited with signal %s (%s)",
                f"[{proc_num}]" if len(termsig) > 1 else "",
                t.name,
                signal.strsignal(t),
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

    if ec == 2 and py_args.ignore_warnings:
        ec = 0

    assert all(
        t is None for t in termsig
    )  # now we should only have had a graceful exit.

    # setup logging based on log level from C++.
    log_level = ["summary", *ipc_info.get("loglevel")]
    rmg_logging.set_logging_level(logger, log_level[-1])

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
