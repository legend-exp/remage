#ifndef _MGLOGGER_HH
#define _MGLOGGER_HH

#include <iostream>
#include <string>

#define ERRLINE_HACK_1(line) #line
#define ERRLINE_HACK_2(line) ERRLINE_HACK_1(line)

#ifdef MGLog
#undef MGLog
#endif
#define MGLog(sev) MGLogger::msg( MGLogger::sev, __FILE__ "(" ERRLINE_HACK_2(__LINE__) ")")

class MGLogger {

  public:

  // a severity enum is defined; use only these values
  enum Severity {debugging=-1, trace=0, routine, warning, error, fatal};
  //  fatal: The message is related to a condition preventing
  //    further execution of the program. ErrLogger will
  //    terminate the program.Programmers should not call
  //    abort or exit themselves.
  //
  //  error: A condition exists such that requested result
  //    or action can not be produced. This is a serious
  //
  //  warning: The result is produced, but may not be
  //    what's desired due to an unexpected condition
  //
  //  routine: Nothing known to be wrong with the result;
  //    messages that are always produced in normal
  //    operation
  //
  //  trace: Messages about the flow of program control
  //    and which optional operations took place.
  //    (This is the default if nothing is defined)
  //
  //  debugging: Information in addition to the above

  static Severity GetSeverity() { return min_severity_; }
  static std::ostream& msg(MGLogger::Severity severity,
                           std::string facility);

  static void SetSeverity(Severity sever) { min_severity_ = sever; }
  static void endlog(std::ostream& s);

  static const std::string GetProjectVersion();

  static void SetColorOutput(bool val) { color_output_ = val; }

  protected:

  MGLogger();
  MGLogger(const MGLogger &);
  ~MGLogger();

  private:

  static std::string to_string(Severity);

  static std::ostream* outstream_;
  static std::ostream* errstream_;
  static std::ostream* nullstream_;

  static Severity min_severity_;

  static bool color_output_;
  static bool do_print_;
  static bool is_fatal_;
};

std::ostream& endlog(std::ostream& s);

#endif
