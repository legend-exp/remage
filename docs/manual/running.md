(manual-running)=

# Running simulations

The `remage` executable drives the simulation from the command line. A short
overview of all available options can be obtained with `remage --help`.

```console
$ remage [OPTIONS] [macros or command string...]
```

## Command line options

Typical invocations supply one or more GDML geometry files and a list of macros
to execute:

```console
$ remage -g setup.gdml -- run.mac
```

:::{note}

The `--` is required here to disambiguate since `-g` can be supplied with
multiple arguments.

:::

The most useful options include:

- `macros` – macro files or inline command strings. If the given argument is not
  an existing file it is interpreted as a newline-separated list of commands.
- `-g, --gdml-files` – include one or more GDML geometry files (see
  {ref}`manual-geometry`).
- `-o, --output-file` – sensitive detector hits output file (see
  {ref}`manual-output`).
- `-i, --interactive` – keep the application open after executing macros and
  present a Geant4 prompt.
- `-t, --threads` – number of worker threads to use (this cannot be combined
  with `-P/--procs`).
- `-P, --procs` – number of worker processes to use (this cannot be combined
  with `-t/--threads`).
- `-w, --overwrite` – overwrite an existing output file.
- `-q, --quiet`/`-v, --verbose`/`-l, --log-level` – control the verbosity.
  Logging levels are `debug`, `detail`, `summary`, `warning`, `error`, `fatal`,
  and `nothing` (e.g. `-l debug`).
- `--version`/`--version-rich` – print version information and exit.
- `-m, --merge-output-files` – merge thread-specific output files at the end of
  execution (see {ref}`manual-output`).
- `--flat-output` – store each Geant4 step as its own row in the output (see
  {ref}`manual-output`).
- `--time-window-in-us` – time window used when reshaping hits (see
  {ref}`manual-output`).
- `-s, --macro-substitutions` – provide `key=value` pairs that will be expanded
  as Geant4 aliases in macros.

## Parallel execution

_remage_ supports two ways of parallelising simulations, each with its own
advantages and limitations.

:::{admonition} In short

If memory is the limiting factor, prefer `--threads`; otherwise, try `--procs`
for better throughput. See below for important notes.

:::

The output from remage running with multiple processes or threads can be merged
as if they were written by a single remage process. This is described in
{ref}`manual-output` in more detail.

:::{warning}

Beware that the performance of the merging `--merge-output-files` together with
`--procs`/`--threads` can take a lot of time as these operations are run on a
single core at the moment.

:::

### Geant4 multithreading

This mode is enabled by passing `--threads INTEGER` to the command line.
_remage_ does not implement its own multithreading but delegates it to Geant4.
This approach is memory-efficient because shared objects (such as the geometry)
are instantiated only once.

:::{warning}

The performance in multithreaded mode does not scale linearly with the number of
threads (see [issue #287](https://github.com/legend-exp/remage/issues/287) for
details).

:::

### Multiple processes

This mode is enabled by passing `--procs INTEGER` to the command line. The
Python wrapper (see the {ref}`dev-guide` for details) can launch several
independent _remage_ instances, each running a single process. This usually
provides near 1:1 performance scaling but is more resource-hungry because every
process carries a full memory footprint.

:::{warning}

In the current design, a macro executed with multiple processes will simulate
more events than a single/multi-threaded mode with the same macro. The total
event count is increased by a factor of the number of processes. Example: with 5
processes and `/run/beamOn 1000` in the macro, _remage_ will simulate 5000
events.

:::

## Batch versus interactive mode

By default `remage` runs all specified macro commands in batch mode and then
exits. When `-i` is given, or the
<project:../rmg-commands.md#rmgmanagerinteractive> command is used in a macro, a
command prompt is opened after macro execution:

```console
$ remage -i -g setup.gdml -- run.mac
remage>
```

## Executing commands in batch mode

Inline macro commands can also be executed directly from the command line,
useful feature when generating macros on the fly (in e.g. a Python script).
Multiple commands may be separated by a newline or passed in multiple arguments:

```console
$ remage -i -g setup.gdml -- "/run/initialize \n /run/beamOn 100"
$ remage -i -g setup.gdml -- "/run/initialize" "/run/beamOn 100"
```

Macro substitutions also work with inline commands. Values are expanded where
`{name}` placeholders appear in the command string:

```console
$ remage -s EVENTS=100 -- "/run/initialize \n /run/beamOn {EVENTS}"
```

## Running from Python

The {mod}`remage` Python package provides the {func}`remage.remage_run`
function, which accepts the same options as the command line interface. This
lets you drive the simulation directly from a Python script:

```python
from remage import remage_run

exit_code, info = remage_run(
    macros="run.mac",
    gdml_files="setup.gdml",
    output="hits.lh5",
    threads=2,
    merge_output_files=True,
)
```

The returned `info` object contains metadata from the C++ executable, including
logging output and locations of temporary files.
