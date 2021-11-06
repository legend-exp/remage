# Contribution guide

Please carefully read this document before contributing to the project and
submitting a pull request.

# Source code

- The Google source code style guidelines
  (https://google.github.io/styleguide/cppguide.html) are adopted, the only
  exception being names of private class data members. In our convention, those
  names are prefixed by an `f`, as in GEANT4.
  ```cpp
  G4int fSomePrivateMember;
  ```

- Class names are generally prefixed with `RMG`. Purely virtual class names
  with `RMGV`.
