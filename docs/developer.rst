Developer's guide
=================

The following rules and conventions have been established for the package
development and are enforced throughout the entire code base. Merge requests
that do not comply to the following directives will be rejected.

To start developing |remage|, fork the remote repository to your personal
GitHub account (see `About Forks
<https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/about-forks>`_).
If you have not set up your ssh keys on the computer you will be working on,
please follow `GitHub's instructions
<https://docs.github.com/en/authentication/connecting-to-github-with-ssh/generating-a-new-ssh-key-and-adding-it-to-the-ssh-agent>`_.
Once you have your own fork, you can clone it via
(replace "yourusername" with your GitHub username):

.. code-block:: console

  $ git clone git@github.com:yourusername/remage.git


Installing dependencies
-----------------------

.. include:: _dependencies.rst

Building
--------

|remage| currently only supports an out-of-tree build using CMake. To build and
install remage from a source checkout, run:

.. code-block:: console

   $ cd remage
   $ mkdir build && cd build
   $ cmake -DCMAKE_INSTALL_PREFIX=<optional prefix> ..
   $ make install

Code style
----------

A set of `pre-commit <https://pre-commit.com>`_ hooks is configured to make
sure that |remage| coherently follows standard coding style conventions.
The pre-commit tool is able to identify common style problems and automatically
fix them, wherever possible. Configured hooks are listed in the
``.pre-commit-config.yaml`` file at the project root folder. They are run
remotely on the GitHub repository through the `pre-commit bot
<https://pre-commit.ci>`_, but should also be run locally before submitting a
pull request:

.. code-block:: console

  $ cd remage
  $ pip install pre-commit
  $ pre-commit run --all-files  # analyse the source code and fix it wherever possible
  $ pre-commit install          # install a Git pre-commit hook (optional, but strongly recommended)

Testing
-------

* The |remage| test suite is available below ``tests/``. We use `ctest
  <https://cmake.org/cmake/help/book/mastering-cmake/chapter/Testing%20With%20CMake%20and%20CTest.html>`_
  to run tests and analyze their output.

* Tests are automatically run for every push event and pull request to the
  remote Git repository on a remote server (currently handled by GitHub
  actions). All pull request typically have to pass all tests before being
  merged. Running the test suite is simple:

  .. code-block:: console

    $ cd remage/build/
    $ make
    $ ctest

  .. note ::

    Some tests are known to be flaky, especially with older Geant4 version
    branches.

Documentation
-------------

We adopt best practices in writing and maintaining |remage|'s
documentation. When contributing to the project, make sure to implement the
following:

* Documentation should be exclusively available on the Project website
  `remage.readthedocs.io <https://remage.readthedocs.io>`_. No READMEs,
  GitHub/LEGEND wiki pages should be written.
* Pull request authors are required to provide sufficient documentation for
  every proposed change or addition.
* General guides, comprehensive tutorials or other high-level documentation
  (e.g. referring to how separate parts of the code interact between each
  other) must be provided as separate pages in ``docs/`` and linked in the
  table of contents.

Writing documentation
^^^^^^^^^^^^^^^^^^^^^

Documentation source files must formatted in reStructuredText (reST). A
reference format specification is available on the `Sphinx reST usage guide
<https://www.sphinx-doc.org/en/master/usage/restructuredtext/index.html>`_.

Automatic command reference generation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you change the Geant macro command-based interface in a pull request, please
regenerate the documentation file :doc:`online command-reference <rmg-commands>`
before committing those changes:

.. code-block:: console

    $ cd remage/build/
    $ make remage-doc-dump

The generated documentation file is also tracked using git, and not automatically
rebuilt when the documentation is built, i.e. for ReadTheDocs.

Manually changing the checked-in file ``rmg-commands.rst`` is neither required nor
possible, all changes will be overwritten by the next automatic update.

The GitHub CI jobs will only check if the command-reference file is up-to-date
with the actual source code for each pull reuquest or push, but it will *not*
update the documentation automatically.

.. |remage| replace:: *remage*
