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

#ifndef _RMG_IPC_HH_
#define _RMG_IPC_HH_

#include <atomic>
#include <string>

/** @brief IPC message sender implementation.
 *  @details \verbatim embed:rst:leading-asterisk
 *  .. note ::
 *      The receiver implementation and message format documentation can be found in
 *      :py:mod:`remage.ipc`.
 *  \endverbatim
 */
class RMGIpc final {

  public:

    RMGIpc() = delete;

    /** @brief Set the IPC pipe (write end) file descriptor.
     */
    static void Setup(int ipc_pipe_fd);

    /** @brief Create an IPC message with a key and a single value.
     */
    static std::string CreateMessage(const std::string& command, const std::string& param) {
      // \x1e (record separator = end of entry)
      // TODO: also implement a CreateMessage variant with \x1f (unit separator = key/value delimiter)
      return command + "\x1e" + param;
    }

    /** @brief Send a non-blocking IPC message.
     *  @details The message is a UTF-8 encoded buffer that already contains the message
     *  structure, such as @ref CreateMessage.
     */
    static bool SendIpcNonBlocking(std::string msg);
    /** @brief Send a blocking IPC message.
     *  @details The message is a UTF-8 encoded buffer that already contains the message
     *  structure, such as @ref CreateMessage.
     */
    static bool SendIpcBlocking(std::string msg);

    /** @brief Flag for waiting for the SIGUSR2 signal for blocking messages.
     *  @details Should not be used by the user and only be set by the signal handler.
     */
    inline static std::atomic<bool> fWaitForIpc = false;

  private:

    inline static int fIpcFd = -1;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
