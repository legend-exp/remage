remage
======

|remage| is a modern C++ simulation framework for germanium experiments.

Building the project
--------------------

Users are required to build the project themselves, since no official binaries
are distributed, at the moment.

Required dependencies
^^^^^^^^^^^^^^^^^^^^^

- `CMake <https://cmake.org>`_ 3.12 or higher
- |geant4|_ 11.0.3 or higher

Optional dependencies
^^^^^^^^^^^^^^^^^^^^^

- |geant4|_ support for:

  - HDF5 object persistency
  - Multithreading
  - GDML geometry description

- |root|_ 6.06 or higher
- |bxdecay0|_ 1.0.10 or higher

.. note::

   Pre-built Docker container images with all necessary dependencies are available `on
   Docker Hub <https://hub.docker.com/repository/docker/gipert/remage-base>`_.

.. note::

   Apptainer images can be easily generated with, e.g.:

   .. code-block:: console

      $ [sudo] apptainer build remage-base_latest.sif docker://gipert/remage-base:latest

   For more details, have a look at `the documentation
   <https://apptainer.org/docs/user/main/build_a_container.html>`_.

Building
^^^^^^^^

.. code-block:: console

   $ git clone https://github.com/legend-exp/remage
   $ cd remage
   $ mkdir build && cd build
   $ cmake -DCMAKE_INSTALL_PREFIX=<optional prefix> ..
   $ make install

Usage
-----

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

    api/index
    Good ol' Doxygen <https://remage.readthedocs.io/en/latest/doxygen/annotated.html>

.. |remage| replace:: *remage*
.. |geant4| replace:: Geant4
.. _geant4: https://geant4.web.cern.ch
.. |root| replace:: ROOT
.. _root: https://root.cern.ch
.. |bxdecay0| replace:: BxDecay0
.. _bxdecay0: https://github.com/BxCppDev/bxdecay0
