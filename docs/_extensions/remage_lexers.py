from __future__ import annotations

import re
from typing import ClassVar

from pygments.lexer import RegexLexer
from pygments.lexers.shell import BashSessionLexer
from pygments.token import Comment, Name, Text
from sphinx.application import Sphinx


class RemageSessionLexer(BashSessionLexer):
    """
    Like BashSessionLexer, but treats "remage> " as the prompt.
    """

    name = "Remage shell session"
    aliases: ClassVar[list[str]] = ["remage"]
    # Primary-prompt regex: match lines that start with “remage> ”
    _ps1rgx = re.compile(r"^(remage> )(.*\n?)")
    # Continuation prompt (if you don't want any special one, just empty)
    _ps2 = ""


class Geant4MacroLexer(RegexLexer):
    name: str = "Geant4Macro"
    aliases: ClassVar[list[str]] = ["geant4", "g4mac"]
    filenames: ClassVar[list[str]] = ["*.mac"]

    tokens: ClassVar[dict] = {
        "root": [
            # comments
            (r"#.*$", Comment),
            # command name at start of line
            (r"^(/\S+)", Name.Function),
            # geant4 alias
            (r"\{\w+\}", Name.Variable),
            # any other text
            (r".", Text),
        ],
    }


def setup(app: Sphinx) -> dict[str, bool]:
    app.add_lexer("remage", RemageSessionLexer)
    app.add_lexer("geant4", Geant4MacroLexer)
    return {"parallel_read_safe": True}
