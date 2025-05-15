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

The {cpp:class}`RMGManager` is crucial for any of your code to extend _remage_.
You can access it via

```c++
#include "RMGManager.hh"
/*...*/
RMGManager manager("myapp", argc, argv);
```

Using the {cpp:class}`RMGManager` you can forward the properties that you
otherwise would have forwarded through CLI arguments:

```c++
manager.IncludeMacroFile(macroName);
manager.SetInteractive(true);
manager.SetOutputFileName(outputfilename);
manager.SetNumberOfThreads(nthreads);
manager.SetOutputOverwriteFiles(overwrite);
```

For all of the possible options check the public functions of the
{cpp:class}`RMGManager`.

## Registering custom classes

To register your own custom C++ classes within _remage_ you can use some
pre-defined methods of _remage_. They are distributed in two places: as class
methods of either {cpp:class}`RMGManager` or {cpp:class}`RMGUserInit`.

```c++
manager.SetUserInit(new MyGeometry());
manager.SetUserInit(new MyPhysicsList());
```

The `SetUserInit()` method supports a detector geometry inheriting from
{cpp:class}`RMGHardware`, a run manager inheriting from `G4RunManager`, a
visualization manager inheriting from `G4VisManager` and a custom physics list
inheriting from `G4VUserPhysicsList`.

If you want to register more custom classes, you will have to use the
{cpp:class}`RMGUserInit` class. Access it with
`auto user_init = RMGManager::Instance()->GetUserInit();`. The obtained instance
now lets you register more custom classes that can customize the simulation flow
more than macro commands:

```c++
user_init->AddSteppingAction<T>(/*...*/);
user_init->AddTrackingAction<T>(/*...*/);
user_init->SetUserGenerator<T>(/*...*/);
user_init->AddOutputScheme<T>(/*...*/);
user_init->AddOptionalOutputScheme<T>("name", /*...*/);
user_init->ActivateOptionalOutputScheme("name");
```

Where you replace `T` with the name of your class and `/*...*/` with the
arguments your class constructor would take when doing `new T(/*...*/)`. Your
stepping action should inherit from `G4UserSteppingAction`, the tracking action
from `G4UserTrackingAction`, the generator from {cpp:class}`RMGVGenerator` and
your custom output scheme from {cpp:class}`RMGVOutputScheme`. For more info
about {cpp:class}`RMGUserInit` check the API docs.

## Starting the run

After setting up your C++ code and classes, you will have to start _remage_. You
do that with

```c++
manager.Initialize();
manager.Run();
```

This means after {cpp:func}`RMGManager::Run` you can add more custom C++ code
that you wish to be run after finishing the simulation. For some examples check
the examples folder.
