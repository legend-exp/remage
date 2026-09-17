from __future__ import annotations

import sys

import h5py

group = sys.argv[1]
plain_file = sys.argv[2]
grouped_file = sys.argv[3]


def get_paths(file: str) -> dict[str, str | None]:
    """Map every object in the file to its soft-link target (None if not a link)."""
    paths: dict[str, str | None] = {}

    with h5py.File(file, "r") as f:

        def visit(name):
            link = f.get(name, getlink=True)
            paths[name] = link.path if isinstance(link, h5py.SoftLink) else None

        f.visit(visit)

    return paths


plain = get_paths(plain_file)
grouped = get_paths(grouped_file)

# the grouped file must hold the same objects, just one level deeper
expected = {group: None} | {
    f"{group}/{name}": (f"/{group}{target}" if target is not None else None)
    for name, target in plain.items()
}

if grouped != expected:
    msg = (
        f"file {grouped_file} does not hold the contents of {plain_file} "
        f"in the /{group} group\n"
        f"missing: {sorted(set(expected) - set(grouped))}\n"
        f"unexpected: {sorted(set(grouped) - set(expected))}"
    )
    raise RuntimeError(msg)
