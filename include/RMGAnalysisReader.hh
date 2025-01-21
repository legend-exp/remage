// Copyright (C) 2022 Luigi Pertoldi <gipert@pm.me>
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

#ifndef _RMG_ANALYSIS_READER_HH_
#define _RMG_ANALYSIS_READER_HH_

#include <string>

#include "G4ThreeVector.hh"
#include "G4VAnalysisReader.hh"

#include "RMGVVertexGenerator.hh"

/**
 * @brief wrapper around \ref(G4VAnalasisReader) instances with special handling for LH5 files.
 *
 * @details notes for threadsafe use:
 * - opening/closing can only be performed on the master thread.
 * - in a multithreaded application, all function calls need to by guarded by a mutex. Worker
 * threads can use the reader
 * - instance after opening, but only one worker can use the reader at a time.
 * - the instance obtained by \ref(GetReader) should only be bound once to variables that are of same
 * storage duration as the \ref(RMGAnalysisReader) instance. Example: If you use a static instance
 * in a class, only bind to static class fields (of the same class) to  read the values into.
 * - it is EXTREMELY important to always use overloads with the ntuple id parameter, and to always
 * pass the result of \ref(GetNtupleID) to them. This affects ntuple reading and column registration.
 */
class RMGAnalysisReader final {

  public:

    RMGAnalysisReader() = default;
    ~RMGAnalysisReader() = default;

    RMGAnalysisReader(RMGAnalysisReader const&) = delete;
    RMGAnalysisReader& operator=(RMGAnalysisReader const&) = delete;
    RMGAnalysisReader(RMGAnalysisReader&&) = delete;
    RMGAnalysisReader& operator=(RMGAnalysisReader&&) = delete;

    /**
     * @brief open an input file for reading of one specific ntuple.
     *
     * @details This function can only be used on the master thread.
     *
     * @param file_name the input file name. the file format is determined from the file extension.
     * @param ntuple_dir_name the first part of the input table name. For the table addressed by
     * "dir/table" this is "dir".
     * @param ntuple_name the first part of the input table name. For the table addressed by
     * "dir/table" this is "table".
     */
    bool OpenFile(std::string& file_name, std::string ntuple_dir_name, std::string ntuple_name);
    /**
     * @brief if any file is open for reading, close the reader.
     *
     * @details this invalidates all reader instances obtained via \ref(GetReader). This function
     * can only be used on the master thread. */
    void CloseFile();

    /**
     * @brief get a pointer to the current underlying G4VAnalysisReader. */
    [[nodiscard]] inline auto GetReader() { return fReader; }
    /**
     * @brief return the ntuple id of the current tuple. If the value is negative, no ntuple is opened. */
    [[nodiscard]] inline auto GetNtupleId() const { return fNtupleId; }
    /**
     * @brief get the file name of the current open file, or an empty string. */
    [[nodiscard]] inline auto& GetFileName() const { return fFileName; }

  private:

    G4VAnalysisReader* fReader = nullptr;
    int fNtupleId = -1;

    std::string fFileName;
    bool fFileIsTemp = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
