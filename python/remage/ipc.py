from __future__ import annotations

import logging
import os
import signal
import subprocess

from ._version import __version__

log = logging.getLogger("remage")


def handle_ipc_message(msg: bytes) -> tuple[bool, list, bool]:
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
    is_fatal = False
    # handle blocking messages, if necessary.
    if msg[0] == "ipc_available":
        if msg[1] != __version__:
            log.error(
                "remage-cpp version %s does not match python-wrapper version %s",
                msg[1],
                __version__,
            )
            is_fatal = True
        msg_ret = None
    elif is_blocking:
        log.warning("Unhandled blocking IPC message %s", str(msg))

    return is_blocking, msg_ret, is_fatal


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

                    is_blocking, unhandled_msg, is_fatal = handle_ipc_message(msg)
                    if unhandled_msg is not None:
                        unhandled_ipc_messages.append(unhandled_msg)
                    if is_fatal:  # the handler wants to stop the app.
                        proc.send_signal(signal.SIGTERM)
                    elif is_blocking:
                        proc.send_signal(signal.SIGUSR2)  # send continuation signal.
    except OSError as e:
        if e.errno == 9:  # bad file descriptor.
            return
        raise e


class IpcResult:
    def __init__(self, ipc_info):
        self.ipc_info = ipc_info

    def get(self, name: str, expected_len: int = 1) -> list[str]:
        msgs = [
            msg[1:]
            for msg in self.ipc_info
            if len(msg) == expected_len + 1 and msg[0] == name
        ]
        if expected_len == 1:
            return [msg[0] for msg in msgs]
        return msgs

    def get_single(self, name: str, default: str) -> str:
        gen = self.get(name)
        if len(gen) > 1:
            msg = f"ipc returned key {name} more than once"
            raise RuntimeError(msg)
        return gen[0] if len(gen) == 1 else default

    def set(self, name: str, values: list[str]) -> None:
        self.remove(name)
        for v in values:
            self.ipc_info.append([name, v])

    def remove(self, name: str) -> None:
        self.ipc_info = [msg for msg in self.ipc_info if msg[0] != name]
