#!/bin/bash

set -euo pipefail

# this install script can manage a remage installation using multiple package managers.
# - uv (the default): manage a venv just for remage and copy the entry point.
# - pip: just execute `python -m pip install --force-reinstall ${RMG_PYTHON_PIP_OPTIONS}`
#        and let the user/conda/... manage their venv themselves.

if [[ "$RMG_PYTHON_PKG_MANAGER" != "uv" && "$RMG_PYTHON_PKG_MANAGER" != "pip" ]]; then
    echo "Package manager '${RMG_PYTHON_PKG_MANAGER}' unknown. Options are pip and uv"
    exit 1
fi

if [[ "$RMG_PYTHON_PKG_MANAGER" == "uv" ]]; then
    # prepare venv with uv.
    ${PYTHON} -m venv "${INSTALL_VENV_DIR}"
    # shellcheck disable=SC1091
    . "${INSTALL_VENV_DIR}/bin/activate"

    python -m pip -q install --no-warn-script-location --upgrade pip
    python -m pip -q install --no-warn-script-location --upgrade uv
fi

# prepare source tree for installation.
cp "${CMAKE_CURRENT_BINARY_DIR}/cpp_config.install.py" "${CMAKE_SOURCE_DIR}/python/remage/cpp_config.py"
export SETUPTOOLS_SCM_PRETEND_VERSION_FOR_REMAGE="${RMG_GIT_VERSION_FULL}"

if [[ "$RMG_PYTHON_PKG_MANAGER" == "uv" ]]; then
    # install our package into this venv
    python -m uv -q pip install --reinstall "${CMAKE_SOURCE_DIR}"
    ln -fs "${INSTALL_VENV_DIR}/bin/remage" "${CMAKE_INSTALL_PREFIX}/bin/remage"

    # create and a user-installable wheel
    python -m uv build --wheel "${CMAKE_SOURCE_DIR}" -o "${CMAKE_INSTALL_PREFIX}/share"

    # move wheel to a version-agnostic file name.
    mv "${CMAKE_INSTALL_PREFIX}/share/"remage-*-py3-none-any.whl "${CMAKE_INSTALL_PREFIX}/share/remage-py3-none-any.whl"
elif [[ "$RMG_PYTHON_PKG_MANAGER" == "pip" ]]; then
    # install the package using the given options into the global/user-managed environment.
    # shellcheck disable=SC2086
    python -m pip install --force-reinstall ${RMG_PYTHON_PIP_OPTIONS} "${CMAKE_SOURCE_DIR}"
fi

# clean-up source tree.
rm "${CMAKE_SOURCE_DIR}/python/remage/cpp_config.py"
