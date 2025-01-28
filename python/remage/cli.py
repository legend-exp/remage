from __future__ import annotations

import logging
import os
import shutil
import signal
import subprocess
import sys
import threading
from pathlib import Path

from .cpp_config import REMAGE_CPP_EXE_PATH

log = logging.getLogger(__name__)


def handle_ipc_message(msg: bytes) -> tuple[bool, list]:
    # parse the IPC message structure.
    is_blocking = False
    msg = msg[0:-1]  # strip trailing ASCII GS ("group separator")
    if msg[-1] == "\x05":  # ASCII ENQ ("enquiry")
        msg = msg[0:-1]
        is_blocking = True
    msg = msg.split("\x1e")  # ASCII RS ("record separator")
    msg = [record.split("\x1f") for record in msg]  # ASCII US ("unit separator")
    msg = [tuple(record) if len(record) > 1 else record[0] for record in msg]

    msg_ret = msg
    # handle blocking messages, if necessary.
    if msg == ["ipc_available", "1"]:
        msg_ret = None
    elif is_blocking:
        log.warning("unhandled blocking IPC message %s", str(msg))

    return is_blocking, msg_ret


def ipc_thread_fn(
    pipe_r: int, proc: subprocess.Popen, unhandled_ipc_messages: list
) -> None:
    try:
        msg_buf = b""
        with os.fdopen(pipe_r, "br", 0) as pipe_file:
            while True:
                line = pipe_file.read(1024)
                if not line:
                    return

                msg_buf += line
                if b"\x1d" not in msg_buf:
                    # not a full message yet.
                    continue

                # handle message buffer.
                for _ in range(msg_buf.count(b"\x1d")):
                    msg_end = msg_buf.index(b"\x1d") + 1
                    # note: switching between binary and strting mode is safe here. ASCII
                    # bytes are binary equal to their utf-8 encoding, and their bytes will
                    # not be part of any utf-8 multibyte sequence.
                    msg = msg_buf[0:msg_end].decode("utf-8")
                    msg_buf = msg_buf[msg_end:]

                    is_blocking, unhandled_msg = handle_ipc_message(msg)
                    if unhandled_msg is not None:
                        unhandled_ipc_messages.append(unhandled_msg)
                    if is_blocking:
                        proc.send_signal(signal.SIGUSR2)  # send continuation signal.
    except OSError as e:
        if e.errno == 9:  # bad file descriptor.
            return
        raise e


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


def remage_cli() -> None:
    ec, ipc_info = _run_remage_cpp()
    if ec not in [0, 2]:
        # remage had an error (::fatal -> ec==134 (SIGABRT); ::error -> ec==1)
        # ec==2 is just a warning, continue in the execution flow.
        sys.exit(ec)

    _log_level = next(
        msg[1] for msg in ipc_info if len(msg) == 2 and msg[0] == "loglevel"
    )
    # TODO: setup logging based on _log_level

    # TODO: further post-processing
    output_files = [msg[1] for msg in ipc_info if len(msg) == 2 and msg[0] == "output"]
    print("output files to merge:", *output_files)  # noqa: T201

    sys.exit(ec)
