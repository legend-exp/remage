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
- |hdf5|_ C++ support for LH5 object persistency

.. note::

   Pre-built Docker container images with all necessary dependencies are available `on
   Docker Hub <https://hub.docker.com/repository/docker/gipert/remage-base>`_.

.. note::

   Apptainer images can be easily generated with, e.g.:

   .. code-block:: console

      $ apptainer build remage-base_latest.sif docker://gipert/remage-base:latest

   For more details, have a look at `the documentation
   <https://apptainer.org/docs/user/main/build_a_container.html>`_.


.. |geant4| replace:: Geant4
.. _geant4: https://geant4.web.cern.ch
.. |root| replace:: ROOT
.. _root: https://root.cern.ch
.. |bxdecay0| replace:: BxDecay0
.. _bxdecay0: https://github.com/BxCppDev/bxdecay0
.. |hdf5| replace:: HDF5
.. _hdf5: https://www.hdfgroup.org/solutions/hdf5
