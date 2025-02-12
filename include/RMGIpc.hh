// Copyright (C) 2025 Manuel Huber
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

#ifndef _RMG_IPC_HH_
#define _RMG_IPC_HH_

#include <atomic>
#include <string>

class RMGIpc final {

  public:

    RMGIpc() = delete;

    static void Setup(int ipc_pipe_fd);

    static std::string CreateMessage(const std::string& command, const std::string& param) {
      // \x1e (record separator = end of entry)
      // TODO: also implement a CreateMessage variant with \x1f (unit separator = key/value delimiter)
      return command + "\x1e" + param;
    }

    static bool SendIpcNonBlocking(std::string msg);
    static bool SendIpcBlocking(std::string msg);

    inline static std::atomic<bool> fWaitForIpc = false;

  private:

    inline static int fIpcFd = -1;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
