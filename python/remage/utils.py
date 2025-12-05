from __future__ import annotations

from collections.abc import Iterable


def _to_list(thing):
    if not isinstance(thing, tuple | list):
        return [thing]
    return thing


def sanitize_macro_cmds(text: str | Iterable[str]) -> list[str]:
    if not isinstance(text, list | tuple):
        text = [text]

    output = []
    for item in text:
        if not isinstance(item, str):
            msg = "macro command must be a string or a collection of strings"
            raise TypeError(msg)

        for line in item.split("\n"):
            cmd = line.strip()
            if cmd != "" and not cmd.startswith("#"):
                output.append(cmd)

    return output
