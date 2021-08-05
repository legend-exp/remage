#include "RMGTools.hh"

#include "RMGLog.hh"

std::tm RMGTools::ToUTCTime(std::chrono::time_point<std::chrono::system_clock> stl_t) {

  auto tt = std::chrono::system_clock::to_time_t(stl_t);
  auto t = gmtime(&tt);
  if (!t) {
    RMGLog::Out(RMGLog::error, "Faled to get the UTC date and time");
    return std::tm(); // copy-elision
  }
  else {
    return std::tm(*t); // copy-elision
  }
}

// vim: shiftwidth=2 tabstop=2 expandtab 
