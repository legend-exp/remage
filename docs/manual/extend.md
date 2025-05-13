# Extending _remage_

Advanced applications can extend _remage_ and link against `libremage` with the
usual CMake syntax:

```cmake
project(myapp)
find_package(remage REQUIRED)
# add_library(myapp ...)
# add_executable(myapp ...)
target_link_libraries(myapp PRIVATE RMG::remage)
```

## Forwarding arguments

The `RMGManager` is crucial for any of your code to extend _remage_. You can
access it via

```c++
#include "RMGManager.hh"
...
RMGManager manager("myapp", argc, argv);
```

Using the `RMGManager` you can forward the properties that you otherwise would
have forwarded through CLI arguments:

```c++
manager.IncludeMacroFile(macroName);
manager.SetInteractive(true);
manager.SetOutputFileName(outputfilename);
manager.SetNumberOfThreads(nthreads);
manager.SetOutputOverwriteFiles(overwrite);
```

For all of the possible options check the public functions of the `RMGManager`
[here]{cpp:class}`RMGManager`.

## Registering custom classes

To register your own custom C++ classes to _remage_ you can use pre-defined
methods of _remage_. They are distributed in two places: as class methods of
either `RMGManager` or `RMGUserInit`.

```c++
manager.SetUserInit(new MyGeometry());
manager.SetUserInit(new MyPhysicsList());
```

The `SetUserInit()` method supports a detector geometry inheriting from
`RMGHardware`, a runmanager inheriting from `G4RunManager`, a vismanager
inheriting from `G4VisManager` and a custom physics list inheriting from
`G4VUserPhysicsList`.

If you want to register more custom classes, you will have to use the
`RMGUserInit` class. Access it with
`auto user_init = RMGManager::Instance()->GetUserInit()`. The `RMGUserInit` now
lets you register more custom classes:

```c++
user_init->AddOutputScheme<T>(...);
user_init->AddSteppingAction<T>(...);
user_init->AddTrackingAction<T>(...);
user_init->SetUserGenerator<T>(...);
user_init->AddOptionalOutputScheme<T>("name", ...);
user_init->ActivateOptionalOutputScheme("name");
```

Where you replace `T` with your class and `...` with the arguments your class
would take when doing `new T(...)`. Your `SteppingAction` should inherit from
`G4UserSteppingAction`, the `TrackingAction` from `G4UserTrackingAction`, the
generator from `RMGVGenerator` and your custom `OutputScheme` from
`RMGVOutputScheme`. For more info about `RMGUserInit` see
[here]{cpp:class}`RMGUserInit`.

## Starting the run

After setting up your C++ code and classes, you will have to start _remage_. You
do that with

```c++
manager.Initialize();
manager.Run();
```

This means after `manager.Run()` you can add more custom C++ code that you wish
to be run after finishing the simulation. For some examples check the examples
folder.
