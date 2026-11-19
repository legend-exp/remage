find_package(Doxygen REQUIRED)

# find remage headers
get_target_property(REMAGE_PUBLIC_HEADER_DIR remage INTERFACE_INCLUDE_DIRECTORIES)
file(GLOB REMAGE_PUBLIC_HEADERS ${REMAGE_PUBLIC_HEADER_DIR}/*.hh)

# The Geant4 Doxygen tag file is only used to cross-link Geant4 classes. The upstream host is
# known to be flaky, so a failed download must not break the docs build: warn and build without
# the cross-links instead.
set(GEANT4_TAG_BASE_URL "https://geant4.kek.jp/Reference/11.3.0")
set(GEANT4_TAG_SHA256 "79b695ba4d8993c6bb8b93aba40fbc019c91378ab0bd1c20e200f8203cfd2b3c")
set(GEANT4_TAG_FILE "${CMAKE_CURRENT_BINARY_DIR}/geant4.tag")

set(_g4tag_ok FALSE)
if(EXISTS "${GEANT4_TAG_FILE}")
  file(SHA256 "${GEANT4_TAG_FILE}" _g4tag_hash)
  if(_g4tag_hash STREQUAL GEANT4_TAG_SHA256)
    set(_g4tag_ok TRUE)
  endif()
endif()

if(NOT _g4tag_ok)
  message(STATUS "Downloading Geant4 tag file from: ${GEANT4_TAG_BASE_URL}/geant4.tag")
  # no EXPECTED_HASH here: with it, a failed download is a fatal configure error.
  file(
    DOWNLOAD "${GEANT4_TAG_BASE_URL}/geant4.tag" "${GEANT4_TAG_FILE}.part"
    TLS_VERIFY OFF # TODO: re-enable
    TIMEOUT 120
    INACTIVITY_TIMEOUT 30
    STATUS _g4tag_status)
  list(GET _g4tag_status 0 _g4tag_code)
  list(GET _g4tag_status 1 _g4tag_status)
  if(_g4tag_code EQUAL 0)
    file(SHA256 "${GEANT4_TAG_FILE}.part" _g4tag_hash)
    if(_g4tag_hash STREQUAL GEANT4_TAG_SHA256)
      file(RENAME "${GEANT4_TAG_FILE}.part" "${GEANT4_TAG_FILE}")
      set(_g4tag_ok TRUE)
    else()
      set(_g4tag_status "SHA256 mismatch (got ${_g4tag_hash})")
    endif()
  endif()
  file(REMOVE "${GEANT4_TAG_FILE}.part")
endif()

set(DOXYGEN_WARN_AS_ERROR "YES")
if(_g4tag_ok)
  set(DOXYGEN_TAGFILES "\"${GEANT4_TAG_FILE}=${GEANT4_TAG_BASE_URL}\"")
else()
  set(DOXYGEN_TAGFILES "")
  set(DOXYGEN_WARN_AS_ERROR "NO")
  string(CONCAT _g4tag_msg "Could not obtain Geant4 Doxygen tag file (${_g4tag_status}). "
                "Building docs without links to the Geant4 reference and with "
                "Doxygen warnings not treated as errors.")
  message(WARNING "${_g4tag_msg}")
  if(DEFINED ENV{GITHUB_ACTIONS})
    # surface as an annotation on the workflow run
    execute_process(COMMAND ${CMAKE_COMMAND} -E echo
                            "::warning title=Geant4 tag file::${_g4tag_msg}")
  endif()
endif()

set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR})
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# Doxygen won't create this for us
file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})

# configure target that runs Doxygen
add_custom_command(
  OUTPUT ${DOXYGEN_INDEX_FILE}
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT} > /dev/null
  DEPENDS ${DOXYFILE_IN} ${REMAGE_PUBLIC_HEADERS}
  MAIN_DEPENDENCY ${DOXYFILE_OUT})

add_custom_target(
  doxygen ALL
  DEPENDS ${DOXYGEN_INDEX_FILE}
  COMMENT "Generating Doxygen docs")

# Sphinx
set(SPHINX_SOURCE ${CMAKE_CURRENT_BINARY_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/_build)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

file(
  GLOB_RECURSE SPHINX_SOURCES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.rst **/*.rst *.md **/*.md *.conf)

foreach(_file ${SPHINX_SOURCES})
  get_filename_component(_dir ${_file} DIRECTORY)
  file(MAKE_DIRECTORY ${SPHINX_SOURCE}/${_dir})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/${_dir} COPYONLY)
endforeach()

file(
  GLOB SPHINX_IMAGES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  _img/*)

file(MAKE_DIRECTORY ${SPHINX_SOURCE}/_img)
foreach(_file ${SPHINX_IMAGES})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/_img COPYONLY)
endforeach()

file(
  GLOB_RECURSE SPHINX_EXTENSIONS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  _extensions/*.py known-warnings.txt)

foreach(_file ${SPHINX_EXTENSIONS})
  get_filename_component(_dir ${_file} DIRECTORY)
  file(MAKE_DIRECTORY ${SPHINX_SOURCE}/${_dir})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/${_dir} COPYONLY)
endforeach()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${SPHINX_SOURCE}/conf.py @ONLY)

get_target_property(PYTHONPATH python-virtualenv PYTHONPATH)

set(_python_src "${PROJECT_SOURCE_DIR}/python/remage")

add_custom_command(
  OUTPUT ${SPHINX_INDEX_FILE}
  COMMAND rm -rf "${SPHINX_SOURCE}/api"
  COMMAND
    ${PYTHONPATH} -m sphinx.ext.apidoc --private --module-first --force --output-dir
    "${SPHINX_SOURCE}/api" "${_python_src}" "${_python_src}/_version.py"
    "${_python_src}/__main__.py"
  COMMAND ${PYTHONPATH} -m sphinx --fail-on-warning -q -b html
          -Dbreathe_projects.remage=${DOXYGEN_OUTPUT_DIR}/xml ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${SPHINX_SOURCE}
  DEPENDS ${SPHINX_SOURCES} ${SPHINX_IMAGES} ${SPHINX_EXTENSIONS} ${DOXYGEN_INDEX_FILE}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in)

# Nice named target so we can run the job easily
add_custom_target(
  sphinx ALL
  DEPENDS ${SPHINX_INDEX_FILE} remage-cli
  COMMENT "Generating Sphinx docs")

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD}/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
