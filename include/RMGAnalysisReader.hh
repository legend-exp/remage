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

#include <map>
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
     * @brief thread-safe access handle to the underlying reader. This handle can be used to set-up
     * ntuple reading (in setup mode) or to read rows from the ntuple. */
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
          fUnits = nullptr;
          if (fLock) { fLock.unlock(); }
        }

        /**
         * @brief wraps @ref G4VAnalysisReader::GetNtupleRow. */
        [[nodiscard]] auto GetNtupleRow() {
          AssertSetup(false);
          return fReader->GetNtupleRow(fNtupleId);
        }
        /**
         * @brief wraps @ref G4VAnalysisReader::SetNtupleDColumn. */
        auto SetNtupleDColumn(const std::string& name, G4double& value,
            const std::vector<std::string>& allowed_units = {}) {
          AssertSetup(true);
          AssertUnit(name, allowed_units);
          return fReader->SetNtupleDColumn(fNtupleId, name, value);
        }
        /**
         * @brief wraps @ref G4VAnalysisReader::SetNtupleFColumn. */
        auto SetNtupleFColumn(const std::string& name, G4float& value,
            const std::vector<std::string>& allowed_units = {}) {
          AssertSetup(true);
          AssertUnit(name, allowed_units);
          return fReader->SetNtupleFColumn(fNtupleId, name, value);
        }
        /**
         * @brief wraps @ref G4VAnalysisReader::SetNtupleIColumn. */
        auto SetNtupleIColumn(const std::string& name, G4int& value,
            const std::vector<std::string>& allowed_units = {}) {
          AssertSetup(true);
          AssertUnit(name, allowed_units);
          return fReader->SetNtupleIColumn(fNtupleId, name, value);
        }

        /**
         * @brief get unit information for the column. an empty string means either no unit
         * attached or no support by the file format. */
        [[nodiscard]] std::string GetUnit(const std::string& name) const;

        /**
         * @brief check whether this access handle is still valid. */
        operator bool() const { return fReader != nullptr && fNtupleId >= 0 && fLock; }

      private:

        // only allow creation or moving in parent.
        inline Access(G4AutoLock lock, G4VAnalysisReader* reader, int nt,
            const std::map<std::string, std::string>* u, bool setup)
            : fReader(reader), fLock(std::move(lock)), fNtupleId(nt), fUnits(u), fCanSetup(setup) {};
        Access(Access&&) = default;

        void AssertUnit(const std::string& name, const std::vector<std::string>& allowed_units) const;
        void AssertSetup(bool setup) const;

        G4VAnalysisReader* fReader = nullptr;
        int fNtupleId = -1;
        const std::map<std::string, std::string>* fUnits;
        G4AutoLock fLock;
        bool fCanSetup = false;
    };

    RMGAnalysisReader() = default;
    ~RMGAnalysisReader() = default;

    RMGAnalysisReader(RMGAnalysisReader const&) = delete;
    RMGAnalysisReader& operator=(RMGAnalysisReader const&) = delete;
    RMGAnalysisReader(RMGAnalysisReader&&) = delete;
    RMGAnalysisReader& operator=(RMGAnalysisReader&&) = delete;

    /**
     * @brief open an input file for reading of one specific ntuple. The return access handle can be
     * used to connect to-be-read column to variables.
     *
     * @details This function can only be used on the master thread. This operation acquires a
     * global lock across all readers that will be held until the access handle is discarded.
     *
     * @param file_name the input file name. the file format is determined from the file extension.
     * @param ntuple_dir_name the first part of the input table name. For the table addressed by
     * "dir/table" this is "dir".
     * @param ntuple_name the first part of the input table name. For the table addressed by
     * "dir/table" this is "table".
     * @param lock a lock instance obtained by @ref RMGAnalysisReader::GetLock - optional.
     * @param force_ext force a file extension/reader type. should be a lowercase file extension
     * like @c lh5, @c hdf5, @c csv, @c root.
     */
    [[nodiscard]] Access OpenFile(const std::string& file_name, std::string ntuple_dir_name,
        std::string ntuple_name, G4AutoLock lock, std::string force_ext = "");
    /**
     * @overload */
    [[nodiscard]] Access OpenFile(const std::string& file_name, std::string ntuple_dir_name,
        std::string ntuple_name, std::string force_ext = "");

    /**
     * @brief if any file is open for reading, close the reader. Also clean-up temporary files.
     *
     * @details This function can only be used on the master thread. This operation acquires a global
     * across all readers. This function will not actually free resources allocated for the reader by Geant4. */
    void CloseFile();

    /**
     * @brief get an access handle to the current underlying G4VAnalysisReader.
     *
     * @details The return access handle can be used to read row(s) from the ntuple. This function
     * acquires a global across all readers that will be held until the access handle is discarded. */
    [[nodiscard]] Access GetLockedReader() const;

    /**
     * @brief acquires a global lock to the analysis reader mutex. */
    [[nodiscard]] G4AutoLock GetLock() const;

    /**
     * @brief get the file name of the current open file, or an empty string. */
    [[nodiscard]] inline auto& GetFileName() const { return fFileName; }

  private:

    static G4Mutex fMutex;

    G4VAnalysisReader* fReader = nullptr;
    int fNtupleId = -1;

    std::map<std::string, std::string> fUnits{};
    bool fHasUnits = false;

    std::string fFileName;
    bool fFileIsTemp = false;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
