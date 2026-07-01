# Agent notes

## Regenerate the command doc dump after messenger changes

`docs/rmg-commands.md` is auto-generated from the C++ messenger command
definitions (`G4GenericMessenger` in `src/*.cc`). Whenever you add, remove, or
change a `/RMG/...` macro command (guidance, parameter name, range, candidates),
regenerate and commit it:

```
cmake --build build --target remage-doc-dump   # or: make -C build remage-doc-dump
git add docs/rmg-commands.md
```

CI (`main.yml`, "Compare checked-in doc dump with current result") diffs the
regenerated file against the committed one and fails if they differ.
