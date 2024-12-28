get_target_property(REMAGE_PYEXE remage-cli PYEXE_PATH)

add_test(NAME python/cli COMMAND "${REMAGE_PYEXE}" --help)
