add_test(
  NAME examples/04-cosmogenics
  WORKING_DIRECTORY $<TARGET_FILE_DIR:04-surface-cosmogenic>
  COMMAND 04-surface-cosmogenic run.mac)
set_tests_properties(examples/04-cosmogenics PROPERTIES LABELS extra)

add_test(
  NAME examples/05-MUSUN
  WORKING_DIRECTORY $<TARGET_FILE_DIR:05-MUSUN>
  COMMAND 05-MUSUN run.mac)
set_tests_properties(examples/05-MUSUN PROPERTIES LABELS extra)

add_test(
  NAME examples/06-NeutronCapture
  WORKING_DIRECTORY $<TARGET_FILE_DIR:06-NeutronCapture>
  COMMAND 06-NeutronCapture runRMGnCapture.mac)
set_tests_properties(examples/06-NeutronCapture PROPERTIES LABELS extra)
