from __future__ import annotations


def _to_list(thing):
    if not isinstance(thing, tuple | list):
        return [thing]
    return thing
