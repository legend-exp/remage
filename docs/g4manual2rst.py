#!/bin/python

"""Convert an output file of remage-doc-dump to a rst file."""

import math
import os
import re
import sys

if len(sys.argv) < 2:
    msg = "need to pass an original file"
    raise ValueError(msg)

path = sys.argv[1]

outlines = [
    "remage macro command reference",
    "=" * 31,
    "",
    "..",
    "    This file is auto-generated by ``make remage-doc-dump`` and should not be edited directly.",
    "    All guidance strings and command info are taken from C++ source files and can be changed there.",
    "",
]
infile = open(path, "rt")
inlines = [line.strip("\n") for line in infile]


def remove_whitespace_lines_end(lines: list):
    for i in range(len(lines) - 1, 0, -1):
        if lines[i].strip() not in ("", "::"):
            break
        del lines[i]


idx = 0
in_cmdblock = False
in_guidance = False
lastlevel = -1
leveldiff = 0

for line in inlines:
    if re.match(r"Command directory path : /RMG/", line):
        line = line.removeprefix("Command directory path : ")
        remove_whitespace_lines_end(outlines)
        outlines.extend(["", line, "-" * len(line), ""])
        in_cmdblock = True
        lastlevel = -1
        leveldiff = 0
    elif re.match(r"Command /RMG/", line):
        line = line.removeprefix("Command ")
        remove_whitespace_lines_end(outlines)
        outlines.extend(["", line, "^" * len(line), ""])
        in_cmdblock = True
        lastlevel = -1
        leveldiff = 0
    elif in_cmdblock and (line == "Guidance :"):
        in_guidance = True
    elif in_cmdblock and in_guidance and not line.startswith(" "):
        if line.startswith("note: "):
            outlines.extend([".. note ::", "", (" " * 4) + line[6:], ""])
        elif line.strip() == "":
            in_guidance = False
        elif line != "":
            outlines.extend([line, ""])
    elif in_cmdblock and line == " Commands : " and not inlines[idx + 1].startswith(" " * 3):
        # ignore directories with no commands.
        pass
    elif in_cmdblock and line == " Sub-directories : " and inlines[idx + 1] == " Commands : ":
        # ignore directories with no commands.
        pass
    elif in_cmdblock and line != "":
        in_guidance = False

        stripped_line = line.lstrip()
        indent = math.ceil((len(line) - len(stripped_line)) / 2)
        if line.startswith(" Range of parameters :"):
            indent = 0
        stripped_line = stripped_line.rstrip()
        if lastlevel == -1 and indent > lastlevel + 1:  # parts of the output have the wrong indentation.
            leveldiff = indent
        indent -= leveldiff
        m = re.match(r"(.*)( [:* ] ?)(.*)?$", line)
        if m:
            g = list(m.groups())
            sep = g[1].strip()
            fmt = "**" if sep == ":" else "``"
            if len(g) > 1:
                g[0] = f"{fmt}{g[0].strip()}{fmt}"
                g[1] = " –"
            if len(g) > 2 and g[2] != "":
                g[2] = f" {g[2].strip()}"
            stripped_line = "".join(g)
        outlines.append("    " * indent + "* " + stripped_line)
        lastlevel = indent
    idx += 1

outfile = open("rmg-commands.rst", "wt")
outfile.writelines([l + "\n" for l in outlines])

print(
    "converted G4 manual",
    os.path.realpath(path),
    "to RST file",
    os.path.realpath("rmg-commands.rst"),
)
