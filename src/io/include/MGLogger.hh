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

/** Logging utilities.  The `MGLog(severity)` function returns an output stream
 * to which log messages should be sent. The message must be terminated with
 * `endlog`.
 * ```
 * MGLog(routine) << "This is a routine message." << endlog;
 * ```
 */
class MGLogger {

  public:

    /** Type of log message.
     * To be specified for every message printed on screen.
     */
    enum Severity {
      debugging = -1, /**< Maximum information about the program flow, technical information about low-level operations */
      trace = 0,      /**< Detailed information about the program flow and which operations took place */
      routine,        /**< Summary information about the program flow, execution of main routines */
      warning,        /**< The result is produced, but may not be what's desired due to an unexpected condition */
      error,          /**< Something went wrong. The issue is serious but the execution might proceed on a different path */
      fatal           /**< The message is related to a condition preventing further execution of the program. `abort()` is automatically called */
    };

    /** Set minimum severity of messages to be actually printed on screen */
    static void SetSeverity(Severity sever) { min_severity_ = sever; }
    /** Get current minimum logging severity */
    static Severity GetSeverity() { return min_severity_; }

    /** Get current version of the software suite */
    static const std::string GetProjectVersion();
    /** Switch to colorized log messages (default = true) */
    static void SetColorOutput(bool val) { color_output_ = val; }

    static std::ostream& msg(MGLogger::Severity severity, std::string facility);
    static void endlog(std::ostream& s);

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

// vim: tabstop=2 shiftwidth=2 expandtab
