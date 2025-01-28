add_test(
  NAME examples/05-MUSUN
  WORKING_DIRECTORY $<TARGET_FILE_DIR:05-MUSUN>
  COMMAND 05-MUSUN run.mac)
set_tests_properties(examples/05-MUSUN PROPERTIES LABELS extra)
