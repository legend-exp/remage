file(REMOVE_RECURSE
  "cpp_config.install.py"
  "/src/python/remage/cpp_config.py"
  "/usr/local/bin/remage"
  "/usr/local/share/remage-py3-none-any.whl"
  "CMakeFiles/install-remage-cli"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/install-remage-cli.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
