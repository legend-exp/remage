file(
  GLOB _aux
  RELATIVE ${PROJECT_SOURCE_DIR}
  *.tex)

# copy them to the build area
foreach(_file ${_aux})
  configure_file(${PROJECT_SOURCE_DIR}/${_file} ${PROJECT_BINARY_DIR}/${_file} COPYONLY)
endforeach()

# copy all files
add_test(
  NAME tex/copy-files
  COMMAND
    /bin/sh -c
    "find ../ -type f \\( -name '*.output*' -o -name '*.tex' \\) ! -path '../tex/*' -exec cp {} . \; "
)
set_tests_properties(tex/copy-files PROPERTIES LABELS tex FIXTURES_SETUP copy-files-fixture)

find_program(PDFLATEX_COMPILER pdflatex)
if(PDFLATEX_COMPILER)
  add_test(NAME tex/compile COMMAND ${PDFLATEX_COMPILER} validation-report.tex)
  set_tests_properties(tex/copy-files PROPERTIES LABELS tex FIXTURES_REQUIRED copy-files-fxiture)

else()
  message(STATUS "No LaTeX compiler found, report cannot be compiled.")
endif()
