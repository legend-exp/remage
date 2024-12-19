# Contributing

Before opening a PR:

- make sure to follow our code style conventions
- run pre-commit and fix all the issues
- add documentation, either as code docstrings or as manual pages
- if unsure about the docs format, build the documentation and check the result
- add tests for your new feature
- run tests and check if they all pass
- if adding a new macro command, update the macro command reference page
- prepare a coherent and tidy commit history (with e.g. `git rebase -i`)

After the CI has run:

- fix all reported problems, if any
- navigate to the ReadTheDocs documentation build (linked in the GitHub actions
  report for the PR) and check that the docs are properly rendered
- download and check the test outputs from the GitHub actions interface

For more details, refer to the
[developer's guide](https://remage.readthedocs.io/en/stable/developer.html).
