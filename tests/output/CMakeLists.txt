set(_visit_hdf5 "false")

# executable for converting remage output HDF5 with ntuples to LH5 tables
if(Geant4_hdf5_FOUND)
  add_executable(test-visit-hdf5 EXCLUDE_FROM_ALL test-visit-hdf5.cc)
  target_include_directories(test-visit-hdf5 PRIVATE ${HDF5_INCLUDE_DIRS})
  target_link_libraries(test-visit-hdf5 PUBLIC ${HDF5_CXX_LIBRARIES})
  set(_visit_hdf5 $<TARGET_FILE:test-visit-hdf5>)
  add_test(NAME output/build-test-visit-hdf5
           COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --config "$<CONFIG>" --target
                   test-visit-hdf5)
endif()

# collect auxiliary files
file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  macros/*.mac macros/*.hdf5ls gdml/*.gdml gdml/*.xml run-test-hdf5.sh)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

set(_macros ntuple-per-det.mac ntuple-flat.mac)

foreach(_mac ${_macros})
  add_test(NAME output/${_mac} COMMAND ./run-test-hdf5.sh $<TARGET_FILE:remage-cli> ${_visit_hdf5}
                                       ${_mac})
endforeach()

list(TRANSFORM _macros PREPEND "output/")
set_tests_properties(${_macros} PROPERTIES LABELS extra DEPENDS output/build-test-visit-hdf5)

# Geant4 <= 11.0.3 is deleting non-empty HDF5 files after a successful run, so disable the tests.
if(Geant4_VERSION VERSION_LESS "11.0.4" OR NOT Geant4_hdf5_FOUND)
  set_tests_properties(${_macros} PROPERTIES DISABLED True)
endif()
