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

#include "G4AutoLock.hh"
#include "G4VAnalysisReader.hh"

#include "RMGConfig.hh"

/**
 * @brief wrapper around @ref G4VAnalysisReader instances with special handling for LH5 files.
 *
 * @details notes for threadsafe use:
 * - opening/closing can only be performed on the master thread.
 * - in a multithreaded application, all function calls are guarded by a mutex. Worker
 * threads can use the reader instance after opening, but only one worker can use the reader at a
 * time.
 * - the reader access handles are generally thread-safe, if no other thread uses any
 * @ref G4VAnalysisReader directly.
 * - the reader should only be bound once to variables that are of static
 * storage duration. Example: only bind to static class fields to  read the values into. Do only
 * unloack the reader access handle after reading/checking the read data.
 */
class RMGAnalysisReader final {

  public:

    /**
     * @brief thread-safe access handle to the underlying reader. */
    class Access final {
        friend class RMGAnalysisReader;

      public:

        ~Access() { unlock(); }

        Access(Access const&) = delete;
        Access& operator=(Access&) = delete;
        Access& operator=(Access const&) = delete;
        Access& operator=(Access&&) = delete;

        /**
         * @brief unlock this access handle before it exits the scope. */
        void unlock() {
          fReader = nullptr;
          fNtupleId = -1;
          if (fLock) { fLock.unlock(); }
        }

        /**
         * @brief wraps @ref G4VAnalysisReader::GetNtupleRow. */
        [[nodiscard]] auto GetNtupleRow() { return fReader->GetNtupleRow(fNtupleId); }
        /**
         * @brief wraps @ref G4VAnalysisReader::SetNtupleDColumn. */
        auto SetNtupleDColumn(const std::string& name, G4double& value) {
          return fReader->SetNtupleDColumn(fNtupleId, name, value);
        }
        /**
         * @brief wraps @ref G4VAnalysisReader::SetNtupleFColumn. */
        auto SetNtupleFColumn(const std::string& name, G4float& value) {
          return fReader->SetNtupleFColumn(fNtupleId, name, value);
        }
        /**
         * @brief wraps @ref G4VAnalysisReader::SetNtupleIColumn. */
        auto SetNtupleIColumn(const std::string& name, G4int& value) {
          return fReader->SetNtupleIColumn(fNtupleId, name, value);
        }

        /**
         * @brief check whether this access handle is still valid. */
        operator bool() const { return fReader != nullptr && fNtupleId >= 0 && fLock; }

      private:

        // only allow creation or moving in parent.
        inline Access(G4AutoLock lock, G4VAnalysisReader* reader, int nt)
            : fLock(std::move(lock)), fReader(reader), fNtupleId(nt) {};
        Access(Access&&) = default;

        G4VAnalysisReader* fReader = nullptr;
        int fNtupleId = -1;
        G4AutoLock fLock;
    };

    RMGAnalysisReader() = default;
    ~RMGAnalysisReader() = default;

    RMGAnalysisReader(RMGAnalysisReader const&) = delete;
    RMGAnalysisReader& operator=(RMGAnalysisReader const&) = delete;
    RMGAnalysisReader(RMGAnalysisReader&&) = delete;
    RMGAnalysisReader& operator=(RMGAnalysisReader&&) = delete;

    /**
     * @brief open an input file for reading of one specific ntuple.
     *
     * @details This function can only be used on the master thread. This operation acquires a
     * global lock to avoid problems.
     *
     * @param file_name the input file name. the file format is determined from the file extension.
     * @param ntuple_dir_name the first part of the input table name. For the table addressed by
     * "dir/table" this is "dir".
     * @param ntuple_name the first part of the input table name. For the table addressed by
     * "dir/table" this is "table".
     */
    [[nodiscard]] Access OpenFile(std::string& file_name, std::string ntuple_dir_name,
        std::string ntuple_name);

    /**
     * @brief if any file is open for reading, close the reader.
     *
     * @details this invalidates all readers obtained via @ref RMGAnalysisReader::GetLockedReader.
     * This function can only be used on the master thread. This operation acquires a global lock to
     * avoid problems. */
    void CloseFile();

    /**
     * @brief get an access handle to the current underlying G4VAnalysisReader.
     *
     * @details this acquires a global lock to avoid problems. The lock is held until the access handle is discarded. */
    [[nodiscard]] Access GetLockedReader() const;

    /**
     * @brief get the file name of the current open file, or an empty string. */
    [[nodiscard]] inline auto& GetFileName() const { return fFileName; }

  private:

    static G4Mutex fMutex;

    G4VAnalysisReader* fReader = nullptr;
    int fNtupleId = -1;

    std::string fFileName;
    bool fFileIsTemp = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
