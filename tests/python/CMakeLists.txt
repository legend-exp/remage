file(
  GLOB _file_list
  RELATIVE ${PROJECT_SOURCE_DIR}
  gdml/*.gdml macros/*.mac *.py)

# copy them to the build area
foreach(_file ${_file_list})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# run all python tests with pytest
add_test(NAME python/all COMMAND "${PYTHONPATH}" -m pytest -vvv)

# run executable tests
# cmake-format: off
add_test(NAME python/cli
    COMMAND "${REMAGE_PYEXE}"
    --gdml-files gdml/geometry.gdml
    --output-file output.lh5
    --overwrite
    --substitute-in-macro ENERGY=1000
    --threads 1
    --log-level summary
    -- macros/run.mac)
# cmake-format: on
