# Developer's guide

The following rules and conventions have been established for the package
development and are enforced throughout the entire code base. Merge requests
that do not comply to the following directives will be rejected.

To start developing _remage_, fork the remote repository to your personal GitHub
account (see
[About Forks](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/about-forks)).
If you have not set up your ssh keys on the computer you will be working on,
please follow
[GitHub's instructions](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent).
Once you have your own fork, you can clone it via (replace "yourusername" with
your GitHub username):

```console
$ git clone git@github.com:yourusername/remage.git
```

## That `remage` executable...

To enhance _remage_'s capabilities without requiring complex C++ code, we
implemented a Python wrapper. This wrapper handles input preprocessing, invokes
the `remage-cpp` executable, and performs output postprocessing. While this
approach slightly complicates the build system, it significantly reduces the
amount of code to write and maintain.

The C++ code resides in the `src` directory, with the `remage-cpp` executable
built from `src/remage.cc`. The Python code is organized as a package under the
`python` directory, where the `cli.py` module provides the _remage_ command-line
interface.

At build time, CMake compiles `remage-cpp` and installs the Python package in
the build area. The Python package and its dependencies (see `pyproject.toml`)
are installed into a virtual environment, ensuring an isolated environment with
all required dependencies. The Python wrapper is configured to use the
`remage-cpp` executable from the build area.

This setup is replicated during installation, targeting the install prefix. A
key advantage of this approach is enabling the use of the _remage_ executable in
unit tests, which run on _remage_ from the build area.

Information about the C++ part of _remage_ is forwarded to the Python wrapper
via the `cmake/cpp_config.py.in` file, which is configured by CMake at build
time and moved into the package source folder.

## Installing dependencies

```{include} _dependencies.md

```

## Building

_remage_ currently only supports an out-of-tree build using CMake. To build and
install remage from a source checkout, run:

```console
$ cd remage
$ mkdir build && cd build
$ cmake -DCMAKE_INSTALL_PREFIX=<optional prefix> ..
$ make install
```

:::{warning}

If you want to run the _remage_ tests the cmake flag `-DBUILD_TESTING=ON` is
required.

:::

:::{note}

A list of available Make targets can be printed by running `make help`.

:::

## Code style

A set of [pre-commit](https://pre-commit.com) hooks is configured to make sure
that _remage_ coherently follows standard coding style conventions. The
pre-commit tool is able to identify common style problems and automatically fix
them, wherever possible. Configured hooks are listed in the
`.pre-commit-config.yaml` file at the project root folder. They are run remotely
on the GitHub repository through the [pre-commit bot](https://pre-commit.ci),
but should also be run locally before submitting a pull request:

```console
$ cd remage
$ pip install pre-commit
$ pre-commit run --all-files  # analyse the source code and fix it wherever possible
$ pre-commit install          # install a Git pre-commit hook (optional, but strongly recommended)
```

## Testing

The _remage_ test suite is available below `tests/`. We use
[ctest](https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html)
to run tests and analyze their output.

Tests are automatically run for every push event and pull request to the remote
Git repository on a remote server (currently handled by GitHub actions). All
pull request typically have to pass all tests before being merged. Running the
test suite locally is simple:

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

:::{tip} Cheatsheet:

```console
$ ctest --print-labels # see all defined test labels
$ ctest -L vis # run only tests with label "vis"
$ ctest -R basics-mt/print-volumes.mac # run only this test
$ ctest -V # show test output
$ ctest --output-on-failure # show test output only if test fails
```

:::

:::{tip}

If you want to open a fancy UI to check the output of `vis` tests, you may
achieve it by:

1. `cd` to `test/confinement`
1. edit `macros/_vis.mac` to make sure you load an interactive UI (e.g.
   `/vis/open OI`)
1. edit the macro you are interested in and swap `_init.mac` with `_vis.mac` at
   the very beginning (after `/control/execute`)
1. run the visualization with
   `remage -i -g gdml/geometry.gdml -- macros/themacro.mac`

:::

### Configuring CMake

You may add a new test with the
[`add_test()`](https://cmake.org/cmake/help/latest/command/add_test.html) CMake
command. Use the `category/test-name` convention to name tests. Here are few
examples:

If the test is supposed to run the `remage` executable, you can use the
`REMAGE_PYEXE`, which stores its path:

```cmake
add_test(NAME basics/test COMMAND ${REMAGE_PYEXE} [<arg>...])
```

If you want to use the low-level `remage-cpp` executable, you can directly use
the `remage-cli-cpp` target:

```cmake
add_test(NAME basics/test COMMAND remage-cli-cpp [<arg>...])
```

If you want to run a Python script, the recommended interpreted to use is the
one from the virtual environment set up in the build area by CMake. Its path is
stored in the `PYTHONPATH` variable:

```cmake
add_test(NAME basics/test COMMAND ${PYTHONPATH} script.py)
set_tests_properties(${_test} PROPERTIES ENVIRONMENT MPLCONFIGDIR=${CMAKE_SOURCE_DIR}/tests)
```

If the Python script is generating an output file (to be e.g. shown in the
validation report), make sure it uses the _remage_ Matplotlib settings by
pointing the `MPLCONFIGDIR` environment variable to the `tests/matplotlibrc`
file:

```cmake
set_tests_properties(${_test} PROPERTIES ENVIRONMENT MPLCONFIGDIR=${CMAKE_SOURCE_DIR}/tests)
```

In case a simulation output file needs to be generated with _remage_, before a
series of tests is ran with Python scripts, one can do so with a fixture:

```cmake
add_test(
  NAME germanium/gen-output
  COMMAND ${REMAGE_PYEXE} -g gdml/geometry.gdml -w -o output.lh5 -- macros/run.mac)
set_tests_properties(germanium/gen-output PROPERTIES FIXTURES_SETUP output-fixture)

# NOTE: you can set all PROPERTIES, including e.g. ENVIRONMENT with one single call to
# set_tests_properties()
add_test(NAME germanium/bremsstrahlung COMMAND ${PYTHONPATH} ./test_brem.py)
set_tests_properties(germanium/bremsstrahlung PROPERTIES FIXTURES_REQUIRED output-fixture)

add_test(NAME germanium/e-range COMMAND ${PYTHONPATH} ./test_e_range.py)
set_tests_properties(germanium/e-range PROPERTIES FIXTURES_REQUIRED output-fixture)
```

For more complete examples, have a look at the existing tests.

## Documentation

We adopt best practices in writing and maintaining _remage_'s documentation.
When contributing to the project, make sure to implement the following:

- Documentation should be exclusively available on the Project website
  [remage.readthedocs.io](https://remage.readthedocs.io). No READMEs,
  GitHub/LEGEND wiki pages should be written.
- Pull request authors are required to provide sufficient documentation for
  every proposed change or addition.
- Documentation for classes/methods etc. should be provided in the same source
  file, as Doxygen-formatted comments (see e.g.
  [this guide](https://www.doxygen.nl/manual/docblocks.html)). They will be
  automatically added to the API documentation section on ReadTheDocs.
- General guides, comprehensive tutorials or other high-level documentation
  (e.g. referring to how separate parts of the code interact between each other)
  must be provided as separate pages in `docs/` and linked in the table of
  contents. These documentation source files must formatted in reStructuredText
  (reST). A reference format specification is available on the
  [Sphinx reST usage guide](https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html).

To generate documentation locally, run

```console
$ cd remage/build/
$ cmake .. -DRMG_BUILD_DOCS=ON
$ make sphinx
```

You'll need a Doxygen installation and Python software dependencies specified in
`docs/environment.yml`. The generated documentation can be viewed by pointing
your browser to `docs/_build/index.html`.

### Cross referencing

Implicit cross-references are automatically generated for all headers until
level 3 (i.e. `###` headers in markdown). The level can be adjusted through the
`myst_heading_anchors = DEPTH` setting in `conf.py`.

To reference a remage macro command auto-generated documentation section type
`<project:./rmg-commands.md#rmgmanagerinteractive>`, which will render as
<project:./rmg-commands.md#rmgmanagerinteractive>.

Further reading on the
[myst-parser documentation](https://myst-parser.readthedocs.io/en/latest/syntax/optional.html#auto-generated-header-anchors).

:::{note}

For robust results, explicit cross-reference targets must be preferred.

:::

:::{tip}

To reference the C++ API docs, use the `{cpp}` domain. For example:

- `` {cpp:class}`RMGHardware` `` becomes {cpp:class}`RMGHardware`
- `` {cpp:func}`RMGHardware::RegisterDetector` `` becomes
  {cpp:func}`RMGHardware::RegisterDetector`
- `` {cpp:func}`RMGGeneratorUtil::IsSampleable` `` becomes
  {cpp:func}`RMGGeneratorUtil::IsSampleable`

A full list of domain roles can be found
[here](https://www.sphinx-doc.org/en/master/usage/domains/cpp.html).

:::

### Code highlighting

Code blocks can be highlighted in myst with the usual markdown syntax. Two
custom lexers are additionally available for the _remage_ documentation. The
first is called `remage` and can be used to highlight _remage_ interactive
prompt text:

:::{code-block}

```remage
remage> /RMG/Geometry/IncludeGDMLFile geometry.gdml
remage> /run/initialize
[Summary -> Checking for overlaps in GDML geometry...
remage> /RMG/Geometry/PrintListOfPhysicalVolumes
[Summary ->  · B00000A  // 0 daugh. // 5.54635 g/cm3  // 488.951 g  // 8.81573 cL  // 1 atm // 293.15 K
[Summary ->  · B00000B  // 0 daugh. // 5.54635 g/cm3  // 700.096 g  // 1.26227 dL  // 1 atm // 293.15 K
...
[Summary ->  · wlsr_ttx // 1 daugh. // 350 mg/cm3 // 1.15983 kg // 3.3138 L   // 1 atm // 293.15 K
[Summary ->  · world    // 1 daugh. // 1e-22 mg/cm3 // 154028 kg // 1.54028e+18 km3 // 3e-18 Pa  // 2.73 K
[Summary ->
[Summary -> Total: 171 volumes
```

:::

The second is called `geant4` and can be used to highlight Geant4 macro command
listings:

:::{code-block}

```geant4
/run/initialize

# create a scene
/vis/open OI
/vis/scene/create
/vis/sceneHandler/attach
```

:::

### Automatic command reference generation

If you change the Geant macro command-based interface in a pull request, please
regenerate the documentation file {doc}`online command-reference <rmg-commands>`
before committing those changes:

```console
$ cd remage/build/
$ make remage-doc-dump
```

The generated documentation file is also tracked using git, and not
automatically rebuilt when the documentation is built, i.e. for ReadTheDocs.

Manually changing the checked-in file `rmg-commands.md` is neither required nor
possible, all changes will be overwritten by the next automatic update.

The GitHub CI jobs will only check if the command-reference file is up-to-date
with the actual source code for each pull reuquest or push, but it will _not_
update the documentation automatically.

### Validation report

Validation reports are automatically generated and uploaded as web pages to
https://legend-exp.github.io/remage/validation for each remage version and for
the `stable` tag of the `remage-base` container image. The source files are
stored in the remage repository below `docs/validation` and can be built locally
through the `sphinx-validation` target:

```console
$ cd remage/build/
$ make sphinx-validation
```

CMake will copy images from the `build/tests` folder at build time and make them
available for the report pages. To include new images to the report, you need to
explicitly list them in the `TESTS_IMAGES` variable defined in
`docs/validation/CMakeLists.txt`:

```cmake
set(TESTS_IMAGES
    confinement/geometrical-and-physical.output.jpeg
    basics/vis-co60.output.jpeg
    ...)
```

:::{note}

PDF images cannot be displayed in the validation report, convert them to bitmap.

:::

:::{warning}

`make sphinx-validation` assumes that the images from the test suite have
already been generated. Make sure to manually run `ctest` before building the
validation report.

:::

:::{note}

The validation report is not deployed for pull requests. Instead, the HTML pages
can be downloaded as GitHub action artifact.

:::
