(manual-containers)=

# Software containers

_remage_ is distributed via pre-built Docker container images. The images are
organized in two layers:

- A **base image** providing all libraries required to build and run _remage_.
  This should be used when developing _remage_.
- The **remage image** that installs the actual _remage_ executable and Python
  package on top of the base image.

## Base image

The Dockerfiles defining the base image are maintained in the
[legendexp_remage-base_img](https://github.com/legend-exp/legendexp_remage-base_img)
repository and are automatically published on
[Docker Hub](https://hub.docker.com/r/legendexp/remage-base).

Each Geant4 version has a corresponding image tag, e.g. `G4.11.2`. Tags suffixed
with `-debuginfo` provide debug symbols useful during development or debugging
of _remage_. Tags suffixed with `-slim` provide lighter images that only include
Geant4 without optional _remage_ dependencies.

`latest` always points to the most recent Geant4 release, while `stable` points
to the previous Geant4 release.

## Remage image

The Dockerfile for the remage image is located in the `.dockerhub/` directory of
this repository. The remage image is built from the `stable` tag of the base
image.

Images are automatically built by Docker Hub for every commit to the main branch
and for each tagged release. They are pushed to
[Docker Hub](https://hub.docker.com/repository/docker/legendexp/remage).

There is a tag for every _remage_ release. `latest` points to the most recent
stable release, whereas `dev` tracks the current state of the `main` branch. If
a tag ends with `-slim`, the corresponding image was built from the `-slim`
variant of the base image.

For typical usage we recommend pulling `legendexp/remage:latest`.
