# Developer's guide

```{admonition} TL;DR / check list
Before opening a PR:

- make sure to follow our code style conventions
- run pre-commit and fix all the issues
- add documentation, either as code docstrings or as manual pages
- if unsure about the docs format, build the documentation and check the result
- add tests for your new feature
- run tests and check if they all pass
- if adding a new macro command, update the macro command reference page
- prepare a coherent and tidy commit history (with e.g. `git rebase -i`)

After the CI has run:

- fix all reported problems, if any
- navigate to the ReadTheDocs documentation build (linked in the GitHub
  actions report for the PR) and check that the docs are properly rendered
- download and check the test outputs from the GitHub actions interface
```

The following rules and conventions have been established for the package
development and are enforced throughout the entire code base. Merge requests
that do not comply to the following directives will be rejected.

To start developing *remage*, fork the remote repository to your personal
GitHub account (see [About Forks](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/about-forks)).
If you have not set up your ssh keys on the computer you will be working on,
please follow [GitHub's instructions](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent).
Once you have your own fork, you can clone it via
(replace "yourusername" with your GitHub username):

```console
$ git clone git@github.com:yourusername/remage.git
```

## Installing dependencies

```{include} _dependencies.md
```

## Building

*remage* currently only supports an out-of-tree build using CMake. To build and
install remage from a source checkout, run:

```console
$ cd remage
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=<optional prefix> ..
$ make install
```

## Code style

A set of [pre-commit](https://pre-commit.com) hooks is configured to make
sure that *remage* coherently follows standard coding style conventions.
The pre-commit tool is able to identify common style problems and automatically
fix them, wherever possible. Configured hooks are listed in the
`.pre-commit-config.yaml` file at the project root folder. They are run
remotely on the GitHub repository through the [pre-commit
bot](https://pre-commit.ci), but should also be run locally before submitting a
pull request:

```console
$ cd remage
$ pip install pre-commit
$ pre-commit run --all-files  # analyse the source code and fix it wherever possible
$ pre-commit install          # install a Git pre-commit hook (optional, but strongly recommended)
```

## Testing

The *remage* test suite is available below `tests/`. We use
[ctest](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html)
to run tests and analyze their output.

Tests are automatically run for every push event and pull request to the
remote Git repository on a remote server (currently handled by GitHub
actions). All pull request typically have to pass all tests before being
merged. Running the test suite locally is simple:

```console
$ cd remage/build/
$ make
$ ctest
```

If tests produce output files with an extension that contains an `.output`
sub-extension, they will be automatically uploaded to GitHub as part of a
tarball. This can be downloaded and inspected from the GitHub actions run page:
navigate to "Actions", select the CI run of interest, scroll to the bottom of
the page.

Cheatsheet:
```console
$ ctest --print-labels # see all defined test labels
$ ctest -L vis # run only tests with label "vis"
$ ctest -R basics-mt/print-volumes.mac # run only this test
```

If you want to open a fanci UI to check the output of `vis` tests, you may
achieve it by:

1. `cd` to `test/confinement`
1. edit `macros/_vis.mac` to make sure you load an interactive UI (e.g. `/vis/open OI`)
1. edit the macro you are interested in and swap `_init.mac` with `_vis.mac` at
   the very beginning (after `/control/execute`)
1. run the visualization with `remage -i -g gdml/geometry.gdml -- macros/themacro.mac`

## Documentation

We adopt best practices in writing and maintaining *remage*'s
documentation. When contributing to the project, make sure to implement the
following:

- Documentation should be exclusively available on the Project website
  [remage.readthedocs.io](https://remage.readthedocs.io). No READMEs,
  GitHub/LEGEND wiki pages should be written.
- Pull request authors are required to provide sufficient documentation for
  every proposed change or addition.
- Documentation for classes/methods etc. should be provided in the same source
  file, as Doxygen-formatted comments (see e.g. [this
  guide](https://www.doxygen.nl/manual/docblocks.html)). They will be
  automatically added to the API documentation section on ReadTheDocs.
- General guides, comprehensive tutorials or other high-level documentation
  (e.g. referring to how separate parts of the code interact between each
  other) must be provided as separate pages in `docs/` and linked in the
  table of contents. These documentation source files must formatted in
  reStructuredText (reST). A reference format specification is available on the
  [Sphinx reST usage guide](https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html).

To generate documentation locally, run

```console
$ cd remage/build/
$ cmake .. -DRMG_BUILD_DOCS=ON
$ make sphinx
```

You'll need a Doxygen installation and Python software dependencies specified
in `docs/environment.yml`. The generated documentation can be viewed by
pointing your browser to `docs/_build/index.html`.

### Automatic command reference generation

If you change the Geant macro command-based interface in a pull request, please
regenerate the documentation file {doc}`online command-reference <rmg-commands>`
before committing those changes:

```console
$ cd remage/build/
$ make remage-doc-dump
```

The generated documentation file is also tracked using git, and not automatically
rebuilt when the documentation is built, i.e. for ReadTheDocs.

Manually changing the checked-in file `rmg-commands.md` is neither required nor
possible, all changes will be overwritten by the next automatic update.

The GitHub CI jobs will only check if the command-reference file is up-to-date
with the actual source code for each pull reuquest or push, but it will *not*
update the documentation automatically.
