# Copyright (C) 2024 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <https://www.gnu.org/licenses/>.

from __future__ import annotations

from collections.abc import Iterable


def _to_list(thing):
    if not isinstance(thing, tuple | list):
        return [thing]
    return thing


def sanitize_macro_cmds(text: str | Iterable[str]) -> list[str]:
    if isinstance(text, str):
        text = [text]
    elif not isinstance(text, list | tuple):
        text = list(text)

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


def proc_name_hash(proc_name: str | bytes) -> int:
    """Helper function to generate FNV-1a process name hashes as in the tracks table.

    .. note::
        This function is only for convenience in analysis of remage output, and is not
        used by remage itself.
    """
    bs = proc_name.encode("utf-8") if isinstance(proc_name, str) else proc_name

    # The following lines are a FNV-1a hash function (based on the CC0 licensed algorithm)
    # see https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    # and http://www.isthe.com/chongo/tech/comp/fnv/index.html for details.
    hash_value = 0x811C9DC5

    for b in bs:
        hash_value ^= b
        hash_value = (hash_value * 0x01000193) & 0xFFFFFFFF

    # xor-fold down to 16 bit.
    return (hash_value >> 16) ^ (hash_value & 0xFFFF)
