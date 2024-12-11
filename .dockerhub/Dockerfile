ARG REMAGE_BASE_FLAVOR="latest"

FROM gipert/remage-base:$REMAGE_BASE_FLAVOR

LABEL maintainer.name="Luigi Pertoldi"
LABEL maintainer.email="gipert@pm.me"

ARG REMAGE_VERSION="dev"
ARG CMAKE_BUILD_TYPE="Release"

USER root
WORKDIR /root

RUN mkdir -p src build /opt/remage && \
    if [ "${REMAGE_VERSION}" = "dev" ]; then \
        rm -rf src && \
        git clone "https://github.com/legend-exp/remage" src && \
        REMAGE_VERSION_SET="$(git -C src describe --tags | sed 's/^v//')"; \
    else \
        REMAGE_VERSION_SET="$(echo $REMAGE_VERSION | sed 's/^v//')" && \
        wget -q -O- "https://github.com/legend-exp/remage/archive/${REMAGE_VERSION}.tar.gz" \
            | tar --strip-components 1 -C src --strip=1 -x -z; \
    fi && \
    cd build && \
    cmake \
        -DCMAKE_INSTALL_PREFIX="/opt/remage" \
        -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
        -DCMAKE_PROJECT_VERSION="${REMAGE_VERSION_SET}" \
        ../src && \
    make -j"$(nproc)" install && \
    if [ "${REMAGE_BASE_FLAVOR}" = "slim" ]; then \
        ctest --output-on-failure --label-exclude 'extra|vis'; \
    else \
        ctest --output-on-failure --label-exclude 'vis'; \
    fi && \
    rm -rf build src

ENV PATH="/opt/remage/bin:$PATH" \
    LD_LIBRARY_PATH="/opt/remage/lib:$LD_LIBRARY_PATH"

ENTRYPOINT ["/opt/remage/bin/remage"]

# vim: ft=dockerfile
