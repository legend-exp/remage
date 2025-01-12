from __future__ import annotations

import shutil
import signal
import subprocess
import sys
from pathlib import Path

from .cpp_config import REMAGE_CPP_EXE_PATH


def _run_remage_cpp() -> int:
    """run the remage-cpp executable and return the exit code as seen in bash."""
    # reuse our own argv[0] to have helpful help messages.
    # but is is expanded (by the kernel?), so find out if we are in $PATH.
    argv = list(sys.argv)
    exe_name = Path(argv[0]).name
    if shutil.which(exe_name) == sys.argv[0]:
        argv[0] = exe_name
    proc = subprocess.Popen(argv, executable=REMAGE_CPP_EXE_PATH)

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
        signal.SIGUSR2,
        signal.SIGWINCH,
    ]

    old_signal_handlers = [signal.signal(sig, new_signal_handler) for sig in signals]

    # wait for C++ executable to finish.
    proc.wait()

    # restore signal handlers again, before running more python code.
    for sig, handler in zip(signals, old_signal_handlers):
        signal.signal(sig, handler)

    return 128 - proc.returncode if proc.returncode < 0 else proc.returncode


def remage_cli() -> None:
    ec = _run_remage_cpp()
    if ec not in [0, 2]:
        # remage had an error (::fatal -> ec==134 (SIGABRT); ::error -> ec==1)
        # ec==2 is just a warning, continue in the execution flow.
        sys.exit(ec)

    # TODO: further post-processing

    sys.exit(ec)
