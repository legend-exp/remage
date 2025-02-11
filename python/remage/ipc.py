from __future__ import annotations

import logging
import os
import signal
import subprocess

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
