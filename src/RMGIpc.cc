// Copyright (C) 2025 Manuel Huber <https://orcid.org/0009-0000-5212-2999>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <https://www.gnu.org/licenses/>.

#include "RMGIpc.hh"

#include <mutex>
#include <poll.h>
#include <unistd.h>

#include "G4Threading.hh"

#include "RMGLog.hh"
#include "RMGVersion.hh"


void RMGIpc::Setup(int ipc_pipe_fd_out, int ipc_pipe_fd_in, int proc_num) {
  if (!G4Threading::IsMasterThread()) {
    RMGLog::OutDev(RMGLog::fatal, "can only be used on the master thread");
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

  const int timeout_ms = 10000; // milliseconds (poll() takes ms)
  // bound the total wait: give up after this many consecutive timeouts, so an unresponsive
  // python-wrapper peer cannot hang the (master) process forever.
  const int max_timeouts = 6;
  int timeouts = 0;
  while (true) {
    int ready = poll(&pfd, 1, timeout_ms);
    if (ready > 0) break; // ACK data is available to read.
    if (ready < 0) {
      if (errno == EINTR) continue; // interrupted by a signal, retry.
      RMGLog::Out(RMGLog::error, "IPC error: poll failed with errno=", errno);
      return false;
    }
    // ready == 0: genuine timeout.
    if (++timeouts >= max_timeouts) {
      RMGLog::Out(RMGLog::error, "IPC error: timed out waiting for ACK from python wrapper");
      return false;
    }
  }
  char ack[2] = "";
  auto acklen = read(fIpcFdIn, ack, sizeof(ack));
  if (acklen != 1 || ack[0] != '\x06') {
    RMGLog::Out(RMGLog::fatal, "IPC error: wrong ACK");
    return false;
  }
  return true;
}

bool RMGIpc::SendIpcNonBlocking(std::string msg) {
  if (fIpcFdOut < 0) return false;

  msg = std::to_string(fProcNum) + "\x1e" + msg;

  msg += "\x1d"; // ASCII GS group separator = end of message.
  if (msg.size() > SSIZE_MAX) {
    RMGLog::Out(RMGLog::error, "IPC message transmit failed for too-large message");
    return false;
  }

  // Serialize writes: multiple worker threads may share the same IPC fd, and a message is only
  // valid as a whole (framed by a trailing \x1d). Interleaving would corrupt the stream.
  static std::mutex write_mutex;
  std::scoped_lock lock(write_mutex);

  // Loop until all bytes are written: a short write would otherwise permanently desync the
  // \x1d-framed stream. Retry on EINTR/EAGAIN.
  size_t written = 0;
  while (written < msg.size()) {
    auto len = write(fIpcFdOut, msg.c_str() + written, msg.size() - written);
    if (len < 0) {
      if (errno == EINTR || errno == EAGAIN) continue;
      RMGLog::Out(RMGLog::error, "IPC message transmit failed with errno=", errno);
      return false;
    }
    written += static_cast<size_t>(len);
  }

  return true;
}

// vim: tabstop=2 shiftwidth=2 expandtab
