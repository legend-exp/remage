#include "MGLogger.hh"
#include "ProjectInfo.hh"

#include <sstream>
#include <cstdlib>

static struct nullstream:
  std::ostream {
    struct nullbuf: std::streambuf {
      int overflow(int c) { return traits_type::not_eof(c); }
    } m_sbuf;
    nullstream(): std::ios(&m_sbuf), std::ostream(&m_sbuf) {}
  } devnull;

MGLogger::Severity MGLogger::min_severity_ = MGLogger::debugging;

std::ostream* MGLogger::outstream_  (&std::cout);
std::ostream* MGLogger::errstream_  (&std::cerr);
std::ostream* MGLogger::nullstream_ (&devnull);

bool MGLogger::do_print_     = true;
bool MGLogger::is_fatal_     = false;
bool MGLogger::color_output_ = true;

MGLogger::MGLogger() {}
MGLogger::MGLogger(const MGLogger&) {}
MGLogger::~MGLogger() {}

std::ostream& MGLogger::msg(MGLogger::Severity severity, std::string facility) {

  do_print_ = true;

  auto n = facility.find("/src/");
  if (n == std::string::npos) {
      std::cerr << "could not find the string '/src/' in the file path " << facility
                << ", this shouldn't happen! Fix this problem in the MGLogger::msg function."
                << std::endl;
  }
  facility.erase(0, n+8);

  if (severity >= min_severity_) {
    if (severity == MGLogger::warning or
        severity == MGLogger::error or
        severity == MGLogger::fatal) {

      if (color_output_) *errstream_ << to_string(severity) << ":\033[1m" << facility << "\033[0m: ";
      else               *errstream_ << to_string(severity) << ":"        << facility << ": ";
    }
    else {
      if (color_output_) *outstream_ << to_string(severity) << ":\033[1m" << facility << "\033[0m: ";
      else               *outstream_ << to_string(severity) << ":"        << facility << ": ";
    }


    if (severity == MGLogger::fatal) is_fatal_ = true;
  }
  else {
    do_print_ = false;
    return *nullstream_ ;
  }

  return *outstream_;
}

// call exit() if last severity is fatal
// this allows to actually print something about the error
// obviously the program does not terminate until endlog is called...
void MGLogger::endlog(std::ostream&) {
  if(do_print_) *outstream_ << std::endl;
  if(is_fatal_) {
      *errstream_ << "Aborting..." << std::endl;
      exit(1);
  }
}

std::ostream& endlog(std::ostream& os){
  MGLogger::endlog(os);
  return devnull;
}

std::string MGLogger::to_string(MGLogger::Severity sever) {
  switch (sever) {
    case -1: return color_output_ ? "\033[35mDebug\033[m"     : "Debug";
    case  0: return color_output_ ? "\033[34mTrace\033[m"     : "Trace";
    case  1: return color_output_ ? "\033[32mRoutine\033[m"   : "Routine";
    case  2: return color_output_ ? "\033[1;33mWarning\033[m" : "Warning";
    case  3: return color_output_ ? "\033[1;31mError\033[m"   : "Error";
    case  4: return color_output_ ? "\033[7;31mFatal\033[m"   : "Fatal";
  }
  return color_output_ ? "\033[7;31mFatal\033[m" : "Fatal";
}

const std::string MGLogger::GetProjectVersion() { return PROJECT_VERSION;  }
