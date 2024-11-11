# remage output files

remage supports multiple output formats (ROOT, HDF5, ...) if it had been
compiled in. The file type to use is selected by the specified output file
name.

The contents of the output files is determined by *output schemes*. An output
scheme does not only contain functionality for the actual output description,
but also might have  parts of Geant4's stacking action functionality. Output
schemes, in general, are remage's way to implement pluggable event selection,
persistency and track stacking.

## Selection of output schemes

Adding a sensitive detector of any type will add the corresponding main output
scheme to the list of active output schemes.

Additional output schemes might be used for **filtering output**. Optional
output schemes can be enabled with the macro command
`/RMG/Manager/ActivateOutputScheme [name]`.

Adding output schemes with C++ code is possible using the `RMGUserInit` system
of remage (access it with `auto user_init =
RMGManager::Instance()->GetUserInit()`:

- `user_init->AddOutputScheme<T>(...);` adds and enables the output scheme `new
  T(...)` on each worker thread,
- `user_init->AddOptionalOutputScheme<T>("name", ...);` adds a name-tag to an
  output scheme, that will not be enabled right away, and
- `user_init->ActivateOptionalOutputScheme("name")` enables such a registered
  output scheme.

## LH5 output

It is possible to directly write a LH5 file from remage, to facilitate reading
output ntuples as a [LH5
Table](https://legend-exp.github.io/legend-data-format-specs/dev/hdf5/#Table).
To use this feature, simply specify an output file with a `.lh5` extension, and
remage will perform the file conversion automatically.

Additionally, the standalone tool `remage-to-lh5` is provided to convert a
default Geant4 HDF5 file to a LH5 file. With this, executing `remage -o
output.lh5 [...]` is roughly equivalent to the combination of commands:

```console
$ remage -o output.hdf5 [...]
$ remage-to-lh5 output.hdf5
$ mv output.{hdf5,lh5}
```

## Physical units in remage output

Geant4 does not allow to attach metadata to columns. The ntuple columns created
by remage contain physical units in their names, encoded as in the
[legend-metadata
spec](https://legend-exp.github.io/legend-data-format-specs/dev/metadata/#Physical-units)
(i.e. adding `_in_<units>` at the end of the name), where the units are
expressed in the typical physical unit symbols. Unfortunately, column names
cannot contain forward slashes, so units lkike `m/s` are not possible to
represent directly. Instead, a backslash (`\`) is used to encode the division
symbol (for example: `velocity_in_m\s`).

This is not really helpful to any consuming application, so `remage-to-lh5`
does a back-transform of the backslashes to forward slashes, and also properly
serializes the units as HDF5 attributes.
