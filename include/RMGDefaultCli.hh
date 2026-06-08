// Copyright (C) 2022 Luigi Pertoldi <https://orcid.org/0000-0002-0467-2571>
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

#ifndef _RMG_DEFAULT_CLI_HH_
#define _RMG_DEFAULT_CLI_HH_

#include <vector>

#include "RMGLog.hh"
#include "RMGManager.hh"

namespace CLI {
  class App;
}
/**
 * @brief The default remage CLI implementation.
 *
 * The virtual functions here can be used as extension points into remage's flow.
 */
class RMGDefaultCli {

  public:

    RMGDefaultCli() = default;
    virtual ~RMGDefaultCli() = default;

    RMGDefaultCli(RMGDefaultCli const&) = delete;
    RMGDefaultCli& operator=(RMGDefaultCli const&) = delete;
    RMGDefaultCli(RMGDefaultCli&&) = delete;
    RMGDefaultCli& operator=(RMGDefaultCli&&) = delete;

    /**
     * @brief Parse the CLI arguments into class fields.
     * @details This calls the @ref SetupCli extension point.
     */
    void ParseCliArgs(int argc, char** argv);
    /** @brief Set-up the CLI11 arguments. */
    virtual void SetupCli(CLI::App&);

    /** @brief Set-up logging from CLI args, setup signal handlers and IPC */
    void SetupLoggingAndIpc();

    /**
     * @brief Set-up and run the actual simulation.
     * @details This calls the @ref SetupRuntime, @ref SetupMacros, @ref SetupOutput and
     * @ref SetupGeometry extension points.
     */
    int RunSimulation(int argc, char** argv);
    /** @brief Set-up runtime properties (e.g., multithreading or -processing). */
    virtual void SetupRuntime(RMGManager& manager);
    /** @brief Load the macro file and substitutions from CLI args */
    virtual void SetupMacros(RMGManager& manager);
    /** @brief Set-up the output file information from CLI args. */
    virtual void SetupOutput(RMGManager& manager);
    /** @brief Load the experimental geometry (by default from GDML ) */
    virtual void SetupGeometry(RMGManager& manager);

  protected:

    int verbose = false;
    bool quiet = false;
    bool version = false;
    bool version_rich = false;
    bool no_banner = false;
    int nthreads = 1;
    int rand_seed = -1;
    bool interactive = false;
    bool overwrite_output = false;
    int pipe_fd_out = -1, pipe_fd_in = -1;
    int proc_num_offset = -1;
    std::vector<std::string> gdmls;
    std::vector<std::string> macros;
    std::vector<std::string> macro_substitutions;
    std::string output;
    RMGLog::LogLevel loglevel = RMGLog::summary;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
