from __future__ import annotations

import logging
import os
import signal
import subprocess

from ._version import __version__

log = logging.getLogger("remage")


def handle_ipc_message(msg: bytes) -> tuple[bool, list, bool]:
    """Parse a raw IPC message from ``remage-cpp``.

    Messages are separated using ASCII control characters.  Each message ends
    with ``GS`` (group separator) and may additionally end in ``ENQ`` (enquiry)
    to indicate that the C++ process expects a response before continuing.
    Records within a message are delimited by ``RS`` (record separator) and each
    record may contain multiple units split by ``US`` (unit separator).  Units
    beyond the first are returned as tuples.

    Parameters
    ----------
    msg
        The raw message bytes including the trailing separator.

    Returns
    -------
    tuple[bool, list, bool]
        ``(is_blocking, parsed_message, is_fatal)`` where ``is_blocking`` is
        ``True`` when the sender waits for a reply, ``parsed_message`` is the
        decoded message or ``None`` if it was consumed internally, and
        ``is_fatal`` signals that the application should terminate.
    """
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
    """Read and handle IPC messages coming from ``remage-cpp``.

    This function runs in a dedicated thread.  It reads from ``pipe_r`` and
    dispatches each complete IPC message to :func:`handle_ipc_message`.  Blocking
    messages are acknowledged by sending ``SIGUSR2`` to the child process, while
    fatal messages trigger ``SIGTERM``.  Any messages that are not handled by
    :func:`handle_ipc_message` are appended to ``unhandled_ipc_messages`` for
    later processing.

    Parameters
    ----------
    pipe_r
        File descriptor for the read end of the IPC pipe.
    proc
        The subprocess running ``remage-cpp``.
    unhandled_ipc_messages
        List that receives messages which were not interpreted here.
    """
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
        """Wrapper around the raw IPC information returned by ``remage-cpp``."""

        self.ipc_info = ipc_info

    def get(self, name: str, expected_len: int = 1) -> list[str]:
        """Return all values of a given key from the IPC message list."""
        msgs = [
            msg[1:]
            for msg in self.ipc_info
            if len(msg) == expected_len + 1 and msg[0] == name
        ]
        if expected_len == 1:
            return [msg[0] for msg in msgs]
        return msgs

    def get_single(self, name: str, default: str) -> str:
        """Return a single value for ``name`` or ``default`` if not present."""
        gen = self.get(name)
        if len(gen) > 1:
            msg = f"ipc returned key {name} more than once"
            raise RuntimeError(msg)
        return gen[0] if len(gen) == 1 else default

    def set(self, name: str, values: list[str]) -> None:
        """Replace existing entries for ``name`` with ``values``."""
        self.remove(name)
        for v in values:
            self.ipc_info.append([name, v])

    def remove(self, name: str) -> None:
        """Remove all records with the given ``name`` from the IPC info."""

        self.ipc_info = [msg for msg in self.ipc_info if msg[0] != name]
