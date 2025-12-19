# Copyright (C) 2024 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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
