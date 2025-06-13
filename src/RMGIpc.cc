// Copyright (C) 2025 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "RMGIpc.hh"

#include <csignal>
#include <unistd.h>

#include "G4Threading.hh"

#include "RMGLog.hh"
#include "RMGVersion.hh"

/// \cond this creates weird namespaces @<long number>
namespace {
  void ipc_signal_handler(int) {
    if (!RMGIpc::fWaitForIpc.is_lock_free()) return;
    RMGIpc::fWaitForIpc = true;
  }
} // namespace
/// \endcond

void RMGIpc::Setup(int ipc_pipe_fd) {
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
  }

  fIpcFd = ipc_pipe_fd;
  if (fIpcFd < 0) return;

  struct sigaction sig{};
  sig.sa_handler = &ipc_signal_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;

  if (sigaction(SIGUSR2, &sig, nullptr) != 0) {
    RMGLog::Out(RMGLog::error, "IPC SIGUSR2 signal handler install failed.");
    fIpcFd = -1;
  }

  // note: this is just a test for the blocking mode.
  if (!SendIpcBlocking(CreateMessage("ipc_available", RMG_PROJECT_VERSION_FULL))) {
    RMGLog::Out(RMGLog::error, "blocking test IPC call failed, disabling.");
    fIpcFd = -1;
  }
}

bool RMGIpc::SendIpcBlocking(std::string msg) {
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
  }
  if (fIpcFd < 0) return false;

  msg += "\x05"; // ASCII ENQ enquiry = ask for continuation.

  sigset_t sig2, sigold;
  sigemptyset(&sig2);
  sigaddset(&sig2, SIGUSR2);

  // Block SIGUSR2 _before_ writing message.
  pthread_sigmask(SIG_BLOCK, &sig2, &sigold);

  if (!SendIpcNonBlocking(msg)) {
    pthread_sigmask(SIG_UNBLOCK, &sig2, nullptr);
    return false;
  }

  // now wait for continuation signal.
  while (!fWaitForIpc) sigsuspend(&sigold);
  fWaitForIpc = false;
  pthread_sigmask(SIG_UNBLOCK, &sig2, nullptr);

  return true;
}

bool RMGIpc::SendIpcNonBlocking(std::string msg) {
  if (fIpcFd < 0) return false;

  msg += "\x1d"; // ASCII GS group separator = end of message.
  if (msg.size() > SSIZE_MAX) {
    RMGLog::Out(RMGLog::error, "IPC message transmit failed for too-large message");
    return false;
  }

  auto len = write(fIpcFd, msg.c_str(), msg.size());

  if (len < 0) {
    RMGLog::Out(RMGLog::error, "IPC message transmit failed with errno=", errno);
    return false;
  }

  if ((size_t)len != msg.size()) {
    // TODO: better handle this case.
    RMGLog::Out(RMGLog::error, "IPC message not fully transmitted. missing=", msg.size() - len);
    return false;
  }

  return true;
}

// vim: tabstop=2 shiftwidth=2 expandtab
