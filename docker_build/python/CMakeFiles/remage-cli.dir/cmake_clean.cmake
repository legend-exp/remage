file(REMOVE_RECURSE
  "../python_venv/bin/remage"
  "cpp_config.build.py"
  "/src/python/remage/cpp_config.py"
  "CMakeFiles/remage-cli"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/remage-cli.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
