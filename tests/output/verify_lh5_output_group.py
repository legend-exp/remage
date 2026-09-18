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

# the group must hold the same objects as the whole plain file, just one level deeper
in_group = {
    name.removeprefix(group + "/"): (
        target.removeprefix("/" + group) if target is not None else None
    )
    for name, target in get_paths(grouped_file).items()
    if name.startswith(group + "/")
}

if in_group != plain:
    msg = (
        f"the /{group} group of {grouped_file} does not hold the contents of {plain_file}\n"
        f"missing: {sorted(set(plain) - set(in_group))}\n"
        f"unexpected: {sorted(set(in_group) - set(plain))}"
    )
    raise RuntimeError(msg)
