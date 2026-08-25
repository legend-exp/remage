ARG REMAGE_BASE_FLAVOR="stable"

FROM legendexp/remage-base:$REMAGE_BASE_FLAVOR

LABEL maintainer.name="Luigi Pertoldi"
LABEL maintainer.email="gipert@pm.me"

# "dev" builds remage from the build context (i.e. the checkout this Dockerfile
# lives in), anything else downloads the corresponding release tarball.
ARG REMAGE_VERSION="dev"

# The version is normally determined by setuptools-scm, but the build context
# is copied in without its git history and release tarballs are unpacked
# without one either, so it has to be supplied explicitly for dev builds:
#
#   docker build --build-arg REMAGE_VERSION_SET="$(setuptools-scm)" .
#
ARG REMAGE_VERSION_SET=""

ARG CMAKE_BUILD_TYPE="Release"

USER root
WORKDIR /root

# only used by dev builds, see below (keep .dockerignore lean, this is the
# whole repository)
COPY . src

RUN if [ "${REMAGE_VERSION}" = "dev" ]; then \
        if [ -z "${REMAGE_VERSION_SET}" ]; then \
            echo "ERROR: dev builds need --build-arg REMAGE_VERSION_SET=<version>" >&2 && \
            exit 1; \
        fi; \
    else \
        rm -rf src && mkdir src && \
        REMAGE_VERSION_SET="$(echo $REMAGE_VERSION | sed 's/^v//')" && \
        wget -q -O- "https://github.com/legend-exp/remage/archive/${REMAGE_VERSION}.tar.gz" \
            | tar --strip-components 1 -C src --strip=1 -x -z; \
    fi && \
    # picked up by both the cmake build (via setuptools-scm) and the pip
    # install below (via hatch-vcs), neither of which can see a git repository
    export SETUPTOOLS_SCM_PRETEND_VERSION="${REMAGE_VERSION_SET}" && \
    mkdir -p build /opt/remage && \
    cd build && \
    cmake \
        -DCMAKE_INSTALL_PREFIX="/opt/remage" \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        ../src && \
    make -j"$(nproc)" install && \
    # NOTE: do not run unit tests, not strictly necessary
    # if [ "${REMAGE_BASE_FLAVOR}" = "slim" ]; then \
    #     ctest --output-on-failure --label-exclude 'extra|vis'; \
    # else \
    #     ctest --output-on-failure --label-exclude 'vis'; \
    # fi && \
    cd .. && \
    # also install the package into the container-provided main venv
    uv --no-cache pip install --upgrade ./src && \
    rm -rf build src && \
    LD_LIBRARY_PATH="/opt/remage/lib:$LD_LIBRARY_PATH" /opt/remage/bin/remage --version # populate numba cache

ENV PATH="/opt/remage/bin:$PATH" \
    LD_LIBRARY_PATH="/opt/remage/lib:$LD_LIBRARY_PATH"

ENTRYPOINT ["/opt/remage/bin/remage"]

# vim: ft=dockerfile
