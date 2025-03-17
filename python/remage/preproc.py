from __future__ import annotations

import logging
import string
import tempfile
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import Any

log = logging.getLogger(__name__)


def process_template_macro(macro_file: str | Path, mapping: Mapping[str, Any]) -> Path:
    """Replace variables in a template macro.

    Parameters
    ----------
    macro_file
        path to template macro file.
    mapping
        `key -> value` mapping of variables and values to be substituted.
    """
    macro_file = Path(macro_file)

    with macro_file.open() as f:
        macro = string.Template(f.read())

    try:
        proc_macro = macro.substitute(mapping)
    except KeyError as e:
        msg = f"missing key '{e.args[0]}' in variable substitution dictionary"
        raise KeyError(msg) from e
    except ValueError as e:
        msg = "invalid template macro format, please check the syntax"
        raise RuntimeError(msg) from e

    tmp_macro_file = tempfile.NamedTemporaryFile(  # noqa: SIM115
        suffix=f"-{macro_file.name}", mode="w", delete=False
    )
    tmp_macro_file.write(proc_macro)
    tmp_macro_file.close()

    return Path(tmp_macro_file.name)


def process_template_macros_in_args(
    cpp_arg_list: Sequence[str],
    template_macros: Sequence[str],
    substitute_args: Sequence[str],
) -> list[str]:
    """Replace template macro paths with substituted macros in remage command line.

    Parameters
    ----------
    cpp_arg_list
        list of remage command line arguments.
    template_macros
        list of template macros present at the end of `cpp_arg_list`.
    substitute_args
        list of key-value variable substitutions (e.g. ``a=1``, ``b=2``, ...)

    Warning
    -------
    This function modifies `cpp_arg_list` in place.
    """
    # parse substitutions dictionary
    substitutions = {kv.split("=")[0]: kv.split("=")[1] for kv in substitute_args}

    # make list of temporary macros with substitutions
    proc_macros = [
        str(process_template_macro(macro, substitutions)) for macro in template_macros
    ]

    # replace macros in the command line with these temporary files
    # the macros are always the last args
    cpp_arg_list[-len(template_macros) :] = proc_macros

    return proc_macros
