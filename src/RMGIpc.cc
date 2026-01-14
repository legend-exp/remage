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

#include <poll.h>
#include <unistd.h>

#include "G4Threading.hh"

#include "RMGLog.hh"
#include "RMGVersion.hh"


void RMGIpc::Setup(int ipc_pipe_fd_out, int ipc_pipe_fd_in, int proc_num) {
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
  }

  if (auto timeout_s = std::getenv("RMG_IPC_TIMEOUT")) {
    fTimeout = std::atoi(timeout_s);
    RMGLog::OutFormat(RMGLog::detail, "setting IPC timeout to {:d} us.", fTimeout);
  }

  fIpcFdOut = ipc_pipe_fd_out;
  fIpcFdIn = ipc_pipe_fd_in;
  fProcNum = proc_num;
  if (fIpcFdOut < 0 || fIpcFdIn < 0) return;

  bool perform_versioncheck = true;
  if (auto check_s = std::getenv("RMG_IPC_DISABLE_VERSION_CHECK")) {
    perform_versioncheck = std::atoi(check_s) > 0;
  }
  // note: this is just a test for the blocking mode.
  if (perform_versioncheck &&
      !SendIpcBlocking(CreateMessage("ipc_available", RMG_PROJECT_VERSION_FULL))) {
    RMGLog::Out(RMGLog::error, "blocking test IPC call failed, disabling.");
    fIpcFdOut = -1;
    fIpcFdIn = -1;
  }
}

bool RMGIpc::SendIpcBlocking(std::string msg) {
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
  }
  if (fIpcFdOut < 0) return false;

  msg += "\x05"; // ASCII ENQ enquiry = ask for continuation.

  if (!SendIpcNonBlocking(msg)) { return false; }

  // wait for result.
  pollfd pfd{.fd = fIpcFdIn, .events = POLLIN, .revents = 0};

  int ready = 0;
  RMGLog::Out(RMGLog::debug, "IPC: poll");
  ready = poll(&pfd, 1, fTimeout);
  while (ready == -1 && errno == EINTR) {
    RMGLog::Out(RMGLog::debug, "IPC: restart poll");
    ready = poll(&pfd, 1, fTimeout);
  }
  if (ready == 1) {
    char ack[2] = "";
    auto acklen = read(fIpcFdIn, ack, sizeof(ack));
    if (acklen != 1 || ack[0] != '\x06') {
      RMGLog::Out(RMGLog::fatal, "IPC error: wrong ACK");
      return false;
    }
    return true;
  } else {
    RMGLog::Out(RMGLog::fatal, "IPC timeout or error");
    return false;
  }
}

bool RMGIpc::SendIpcNonBlocking(std::string msg) {
  if (fIpcFdOut < 0) return false;

  msg = std::to_string(fProcNum) + "\x1e" + msg;

  msg += "\x1d"; // ASCII GS group separator = end of message.
  if (msg.size() > SSIZE_MAX) {
    RMGLog::Out(RMGLog::error, "IPC message transmit failed for too-large message");
    return false;
  }

  auto len = write(fIpcFdOut, msg.c_str(), msg.size());

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
