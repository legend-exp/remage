remage
======

|remage| is a modern C++ simulation framework for germanium experiments.

Pre-built binaries
------------------

The recommended and fastest way of running |remage| is through pre-built
software containers. Stable releases are regularly made available `on Docker
Hub <https://hub.docker.com/repository/docker/gipert/remage>`_. To obtain and
run the latest just do:

.. code-block:: console

   $ docker run gipert/remage --help # just prints a help message

If you prefer `Apptainer <https://apptainer.org/>`_, you can easily generate an image locally:

.. code-block:: console

  $ apptainer build remage_latest.sif docker://gipert/remage:latest
  $ apptainer run remage_latest.sif --help

If containers do not work for you, see the next section to learn how to build
and install from source.

Building from source
--------------------

In preparation for the actual build, users are required to obtain some
dependencies.

.. include:: _dependencies.rst

Building
^^^^^^^^

The build system is based on CMake:

.. code-block:: console

   $ git clone https://github.com/legend-exp/remage
   $ cd remage
   $ mkdir build && cd build
   $ cmake -DCMAKE_INSTALL_PREFIX=<optional prefix> ..
   $ make install

Quick start
-----------

.. warning::
   A proper user guide is not available yet. In the meanwhile, users can have a
   look at the `examples
   <https://github.com/legend-exp/remage/tree/main/examples>`_.

In the simplest application, the user can simulate in an existing GDML geometry
through the ``remage`` executable:

.. code-block:: console

   $ remage --help
   remage: simulation framework for germanium experiments
   Usage: ./src/remage [OPTIONS] [macros...]

   Positionals:
     macros FILE ...             Macro files

   Options:
     -h,--help                   Print this help message and exit
     -q                          Print only warnings and errors
     -v [0]                      Increase verbosity
     -l,--log-level LEVEL [summary]
                                 Logging level debug|detail|summary|warning|error|fatal|nothing
     -i,--interactive            Run in interactive mode
     -t,--threads INT            Number of threads
     -g,--gdml-files FILE ...    GDML files
     -o,--output-file FILE       Output file for detector hits


Macro files can use all available upstream Geant4 macro commands, as well as the
:doc:`remage macro interface <rmg-commands>`.

Advanced applications can extend |remage| and link against ``libremage`` with the
usual CMake syntax:

.. code-block:: cmake

    project(myapp)
    find_package(remage)
    # add_library(myapp ...)
    # add_executable(myapp ...)
    target_link_libraries(myapp PRIVATE RMG::remage)

Next steps
----------

.. toctree::
    :maxdepth: 2

    Geant4 command interface <rmg-commands>
    Output structure & LH5 output <output>

.. toctree::
    :maxdepth: 1
    :caption: Development

    Developer's guide <developer>
    api/index
    Good ol' Doxygen <https://remage.readthedocs.io/en/latest/doxygen/annotated.html>
    Source Code <https://github.com/legend-exp/remage>
    License <https://github.com/legend-exp/remage/blob/main/LICENSE>
    Citation <https://doi.org/10.5281/zenodo.11115662>

.. |remage| replace:: *remage*
