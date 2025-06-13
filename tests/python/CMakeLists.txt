add_test(NAME python/cli COMMAND "${REMAGE_PYEXE}" -q --help)

add_test(NAME python/pytest-wrapper COMMAND ${PYTHONPATH} -m pytest -vvv
                                            ${CMAKE_CURRENT_SOURCE_DIR})
