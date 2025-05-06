# Extending _remage_

:::{todo}
Add more information.
:::

Advanced applications can extend _remage_ and link against `libremage` with the
usual CMake syntax:

```cmake
project(myapp)
find_package(remage REQUIRED)
# add_library(myapp ...)
# add_executable(myapp ...)
target_link_libraries(myapp PRIVATE RMG::remage)
```
