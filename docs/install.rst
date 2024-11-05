Installation
============

Pre-built binaries
------------------

The recommended and fastest way of running |remage| is through pre-built
software containers. Stable releases are regularly made available `on Docker
Hub <https://hub.docker.com/repository/docker/legendexp/remage>`_. To obtain and
run the latest just do:

.. code-block:: console

   $ docker run legendexp/remage:latest --help # just prints a help message

If you prefer `Apptainer <https://apptainer.org/>`_, you can easily generate an image locally:

.. code-block:: console

  $ apptainer build remage_latest.sif docker://legendexp/remage:latest
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

.. |remage| replace:: *remage*
