# Sphinx
set(SPHINX_SOURCE ${CMAKE_CURRENT_BINARY_DIR})
set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/_build)

# find and copy docs source files
file(
  GLOB_RECURSE SPHINX_SOURCES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  *.rst **/*.rst *.md **/*.md)

foreach(_file ${SPHINX_SOURCES})
  get_filename_component(_dir ${_file} DIRECTORY)
  file(MAKE_DIRECTORY ${SPHINX_SOURCE}/${_dir})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/${_dir} COPYONLY)
endforeach()

# create an (empty) build directory to satisfy the install rule below if the validation suite
# has not been built yet.
file(MAKE_DIRECTORY ${SPHINX_BUILD})

# find and copy docs image files
file(
  GLOB SPHINX_IMAGES
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  _img/*)

file(MAKE_DIRECTORY ${SPHINX_SOURCE}/_img)
foreach(_file ${SPHINX_IMAGES})
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${_file} ${SPHINX_SOURCE}/_img COPYONLY)
endforeach()

# now we want to add also images from the test suite. need to make an
# (explicit) list of images to pick from the test suite
# TODO: add new images here
# cmake-format: off
set(TESTS_IMAGES
    bxdecay0/double-beta-e-combined-primary-energy.output.png
    bxdecay0/double-beta-e-primary-opening-angle.output.png
    confinement/native-surface.output.jpeg
    confinement/native-volume.output.jpeg
    confinement/complex-surface.output.jpeg
    confinement/complex-volume.output.jpeg
    confinement/geometrical.output.jpeg
    confinement/geometrical-or-physical.output.jpeg
    confinement/geometrical-and-physical.output.jpeg
    confinement/surface-sample-bounding-box-simple.output.jpeg
    confinement/surface-sample-bounding-box-subtraction.output.jpeg
    confinement/surface-sample-bounding-box-union.output.jpeg
    confinement/vis-surface-box.output.jpeg
    confinement/vis-surface-con.output.jpeg
    confinement/vis-surface-sub.output.jpeg
    confinement/vis-surface-tubby.output.jpeg
    confinement/vis-surface-uni.output.jpeg
    confinement/vis-surface-trd.output.jpeg
    confinement/with-daughters.output.jpeg
    confinement/confinement.simple-solids-surface-box-3d.output.png
    confinement/confinement.simple-solids-surface-con-3d.output.png
    confinement/confinement.simple-solids-surface-sub-3d.output.png
    confinement/confinement.simple-solids-surface-tubby-3d.output.png
    confinement/confinement.simple-solids-surface-uni-3d.output.png
    confinement/confinement.simple-solids-surface-trd-3d.output.png
    confinement/confinement.simple-solids-surface-box-projections.output.png
    confinement/confinement.simple-solids-surface-con-projections.output.png
    confinement/confinement.simple-solids-surface-sub-projections.output.png
    confinement/confinement.simple-solids-surface-tubby-projections.output.png
    confinement/confinement.simple-solids-surface-uni-projections.output.png
    confinement/confinement.simple-solids-surface-trd-projections.output.png
    confinement/confinement.simple-solids-surface-box-ratios.output.png
    confinement/confinement.simple-solids-surface-con-ratios.output.png
    confinement/confinement.simple-solids-surface-sub-ratios.output.png
    confinement/confinement.simple-solids-surface-tubby-ratios.output.png
    confinement/confinement.simple-solids-surface-uni-ratios.output.png
    confinement/confinement.simple-solids-surface-trd-ratios.output.png
    confinement/vis-ge-array.output.jpeg
    confinement/relative-ge.output.png
    confinement/lar-in-check-ratios.output.png
    confinement/lar-in-check-xy.output.png
    confinement/lar-in-check-xz.output.png
    confinement/lar-sub-check-xy.output.png
    confinement/lar-sub-check-xz.output.png
    confinement/lar-int-and-sub-check-xy.output.png
    confinement/lar-int-and-sub-check-xz.output.png
    distances/distance-ge-poststep.output.png
    distances/distance-ge-prestep.output.png
    distances/distance-ge-average.output.png
    observables/vis-observables-ge-gdml.output.jpeg
    nist/e-range-ge-distributions.output.png
    nist/e-range-ge-vs-estar.output.png
    nist/e-stopping-power-ge-vs-estar.output.png
    observables/tracks-bulk-10um-limit.tracks.out0.png
    observables/tracks-bulk-10um-limit.tracks.out1.png
    observables/tracks-bulk-10um-limit.tracks.out2.png
    observables/tracks-bulk-no-limit.tracks.out0.png
    observables/tracks-bulk-no-limit.tracks.out1.png
    observables/tracks-bulk-no-limit.tracks.out2.png
    observables/tracks-bulk-10um-limit.step-hist.png
    observables/tracks-bulk-10um-limit.step-vs-dist.png
    observables/tracks-bulk-10um-limit.step-vs-dist-zoom.png
    observables/tracks-bulk-no-limit.step-hist.png
    observables/tracks-bulk-no-limit.step-vs-dist.png
    observables/tracks-bulk-no-limit.step-vs-dist-zoom.png
    observables/beta-observables.bulk-active-energy.eff.output.png
    observables/beta-observables.bulk-active-energy.spec.output.png
    observables/beta-observables.bulk-r90.eff.output.png
    observables/beta-observables.bulk-r90.spec.output.png
    observables/beta-observables.bulk-total-energy.eff.output.png
    observables/beta-observables.bulk-total-energy.spec.output.png
    observables/beta-observables.not-tl-active-energy.eff.output.png
    observables/beta-observables.not-tl-active-energy.spec.output.png
    observables/beta-observables.tl-active-energy.eff.output.png
    observables/beta-observables.tl-active-energy.spec.output.png
    observables/beta-observables.surf-total-energy.eff.output.png
    observables/beta-observables.surf-total-energy.spec.output.png
    observables/beta-observables.surf-active-energy.eff.output.png
    observables/beta-observables.surf-active-energy.spec.output.png
    observables/beta-observables.surf-max-z.eff.output.png
    observables/beta-observables.surf-max-z.spec.output.png
    processes/innerbrem-gamma-vertex-energy.output.png
    processes/innerbrem-signal.output.png
    processes/innerbrem-difference.output.png
    performance/combine-tracks-benchmark.output.png)

# cmake-format: on

# copy them over, but at build time!
set(COPIED_IMGS)
foreach(_file ${TESTS_IMAGES})
  set(_src ${CMAKE_BINARY_DIR}/tests/${_file})
  set(_dest ${CMAKE_CURRENT_BINARY_DIR}/_img/${_file})
  add_custom_command(
    OUTPUT ${_dest}
    COMMAND ${CMAKE_COMMAND} -E copy ${_src} ${_dest}
    DEPENDS ${_src})
  list(APPEND COPIED_IMGS ${_dest})
endforeach()

add_custom_target(
  copy_files ALL
  DEPENDS ${COPIED_IMGS}
  COMMENT "Copying images from test suite")

# configure conf.py
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in ${SPHINX_SOURCE}/conf.py @ONLY)

get_target_property(PYTHONPATH python-virtualenv PYTHONPATH)

add_custom_command(
  OUTPUT ${SPHINX_BUILD}/index.html
  COMMAND ${PYTHONPATH} -m sphinx --fail-on-warning -q -b html ${SPHINX_SOURCE} ${SPHINX_BUILD}
  WORKING_DIRECTORY ${SPHINX_SOURCE}
  DEPENDS ${SPHINX_SOURCES} ${SPHINX_IMAGES} ${DOXYGEN_INDEX_FILE} ${COPIED_IMGS}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in)

# Nice named target so we can run the job easily
add_custom_target(
  sphinx-validation
  DEPENDS ${SPHINX_BUILD}/index.html remage-cli
  COMMENT "Generating validation report with Sphinx")

add_custom_command(
  OUTPUT ${SPHINX_BUILD}/latex/remagesvalidationsuite.tex
  COMMAND ${PYTHONPATH} -m sphinx --fail-on-warning -q -b latex ${SPHINX_SOURCE}
          ${SPHINX_BUILD}/latex
  WORKING_DIRECTORY ${SPHINX_SOURCE}
  DEPENDS ${SPHINX_SOURCES} ${SPHINX_IMAGES} ${DOXYGEN_INDEX_FILE} ${COPIED_IMGS}
  MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
  ${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in)

# Nice named target so we can run the job easily
add_custom_target(
  sphinx-validation-latex
  DEPENDS ${SPHINX_BUILD}/latex/remagesvalidationsuite.tex
  COMMENT "Generating LaTeX validation report with Sphinx")

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD}/ DESTINATION ${CMAKE_INSTALL_DOCDIR}/validation)
