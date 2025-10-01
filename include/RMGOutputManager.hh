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

#ifndef _RMG_OUTPUT_MANAGER_HH_
#define _RMG_OUTPUT_MANAGER_HH_

#include <atomic>
#include <memory>
#include <set>
#include <vector>

#include "G4AnalysisManager.hh"
#include "G4Threading.hh"
#include "globals.hh"

#include "RMGLog.hh"

class G4GenericMessenger;
/**
 * @brief Manages output operations including ntuple registration and persistent storage.
 *
 * This singleton class provides methods for configuring output options,
 * registering ntuples, and handling file persistency for analysis purposes.
 */
class RMGOutputManager {

  public:

    RMGOutputManager();
    ~RMGOutputManager() = default;

    RMGOutputManager(RMGOutputManager const&) = delete;
    RMGOutputManager& operator=(RMGOutputManager const&) = delete;
    RMGOutputManager(RMGOutputManager&&) = delete;
    RMGOutputManager& operator=(RMGOutputManager&&) = delete;

    // getters
    /**
     * @brief Gets the singleton instance of RMGOutputManager.
     *
     * Creates a new instance if none exists.
     * @return Pointer to the RMGOutputManager instance.
     */
    static RMGOutputManager* Instance() {
      if (!fRMGOutputManager) fRMGOutputManager = new RMGOutputManager();
      return fRMGOutputManager;
    }

    /**
     * @brief Checks if persistency is enabled.
     * @return true if output persistency is enabled.
     */
    [[nodiscard]] bool IsPersistencyEnabled() const { return fIsPersistencyEnabled; }
    /**
     * @brief Retrieves the output file name.
     * @return Reference to the output file name string.
     */
    const std::string& GetOutputFileName() { return fOutputFile; }
    /**
     * @brief Checks if the output file name is set to "none".
     * @return true if output file name equals "none".
     */
    [[nodiscard]] bool HasOutputFileNameNone() const { return fOutputFile == OUTPUT_FILE_NONE; }
    /**
     * @brief Checks if a valid output file name is set.
     * @return true if output file name is non-empty and not "none".
     */
    [[nodiscard]] bool HasOutputFileName() const {
      return !fOutputFile.empty() && fOutputFile != OUTPUT_FILE_NONE;
    }
    /**
     * @brief Indicates whether output files should be overwritten.
     * @return true if overwriting is enabled.
     */
    [[nodiscard]] bool GetOutputOverwriteFiles() const { return fOutputOverwriteFiles; }
    /**
     * @brief Gets the directory name for output ntuples.
     * @return Reference to the ntuple directory name.
     */
    const std::string& GetOutputNtupleDirectory() { return fOutputNtupleDirectory; }
    /**
     * @brief Checks if output ntuples are generated per detector.
     * @return true if ntuples are generated per detector.
     */
    [[nodiscard]] bool GetOutputNtuplePerDetector() const { return fOutputNtuplePerDetector; }
    /**
     * @brief Checks if volume names are used for output ntuple naming.
     * @return true if volume names are used.
     */
    [[nodiscard]] bool GetOutputNtupleUseVolumeName() const { return fOutputNtupleUseVolumeName; }

    /**
     * @brief Gets the directory where temporary output files (e.g. .hdf5 for .lh5) are written.
     * @return Reference to the temp directory name (empty string means use output file directory).
     */
    const std::string& GetOutputTempDirectory() { return fOutputTempDirectory; }

    /**
     * @brief Retrieves the set of registered ntuple detector identifiers.
     * @return Set of detector identifiers.
     */
    const std::map<int, std::pair<int, std::string>>& GetNtupleIDs() { return fNtupleIDs; }
    /**
     * @brief Retrieves the set of registered auxiliary ntuple identifiers.
     * @return Set of auxiliary ntuple names.
     */
    std::set<std::string> GetAuxNtupleNames() {
      std::set<std::string> nt_names;
      for (auto const& [k, v] : fNtupleAuxIDs) nt_names.insert(k);
      return nt_names;
    }

    // setters
    /**
     * @brief Enables or disables output persistency.
     * @param flag Set to true to enable, false to disable.
     */
    void EnablePersistency(bool flag = true) { fIsPersistencyEnabled = flag; }

    /**
     * @brief Sets the output file name.
     * @param filename The name of the output file.
     */
    void SetOutputFileName(std::string filename) { fOutputFile = filename; }
    /**
     * @brief Configures whether output files should be overwritten.
     * @param overwrite True to enable overwriting, false otherwise.
     */
    void SetOutputOverwriteFiles(bool overwrite) { fOutputOverwriteFiles = overwrite; }
    /**
     * @brief Sets the directory for output ntuples.
     * @details this might not be used by all output file formats.
     * @param dir The directory name for ntuple output.
     */
    void SetOutputNtupleDirectory(std::string dir) { fOutputNtupleDirectory = dir; }

    /**
     * @brief Sets a custom directory for temporary output files.
     * @details If empty, temporary files are created next to the final output file.
     * @param dir The directory path for temporary output.
     */
    void SetOutputTempDirectory(std::string dir) { fOutputTempDirectory = dir; }

    /**
     * @brief Registers an alreaday created ntuple for a given detector.
     * @param det_uid Unique identifier for the detector.
     * @param ntuple_id Identifier for the ntuple.
     * @param table_name name of the the ntuple on disk.
     *
     * @return The registered ntuple identifier.
     */
    int RegisterNtuple(int det_uid, int ntuple_id, std::string table_name);
    /**
     * @brief Creates and registers a ntuple for a given detector.
     *
     * @details An ordinary ntuple that stores information related to stepping
     * data. An IPC message keyed as "output_ntuple" is automatically sent to
     * communicate the output scheme name and the output table name.
     *
     * @param det_uid Unique identifier for the detector.
     * @param table_name Name of the table.
     * @param oscheme Type name of the output scheme registering this ntuple.
     * @param ana_man Pointer to the analysis manager.
     * @return The created ntuple identifier.
     */
    int CreateAndRegisterNtuple(
        int det_uid,
        std::string table_name,
        std::string oscheme,
        G4AnalysisManager* ana_man
    );
    /**
     * @brief Creates and registers an auxiliary ntuple.
     *
     * @details An auxiliary ntuple stores information not strictly related to
     * stepping data. An IPC message keyed as "output_ntuple_aux" is automatically
     * sent to communicate the output scheme name and the output table name.
     *
     * @param table_name Name of the output table.
     * @param oscheme Type name of the output scheme registering this ntuple.
     * @param ana_man Pointer to the analysis manager.
     * @return The created auxiliary ntuple identifier.
     */
    int CreateAndRegisterAuxNtuple(
        std::string table_name,
        std::string oscheme,
        G4AnalysisManager* ana_man
    );
    /**
     * @brief Gets the ntuple identifier for a given detector.
     * @param det_uid Unique identifier for the detector.
     * @return The ntuple identifier.
     */
    int GetNtupleID(int det_uid) { return fNtupleIDs[det_uid].first; }
    /**
     * @brief Gets the auxiliary ntuple identifier for a given key.
     * @param det_uid Key for the auxiliary ntuple.
     * @return The auxiliary ntuple identifier.
     */
    int GetAuxNtupleID(std::string det_uid) { return fNtupleAuxIDs[det_uid]; }

    /**
     * @brief Activates an optional output scheme.
     * @param name Name of the output scheme to activate.
     */
    // NOLINTNEXTLINE(readability-make-member-function-const)
    void ActivateOptionalOutputScheme(std::string name);

  private:

    static inline const std::string OUTPUT_FILE_NONE = "none";
    std::string fOutputFile;
    bool fIsPersistencyEnabled = true;
    bool fOutputOverwriteFiles = false;
    bool fOutputNtuplePerDetector = true;
    bool fOutputNtupleUseVolumeName = false;
    std::string fOutputNtupleDirectory = "stp";
    std::string fOutputTempDirectory; // optional override for temporary files location

    /** @brief Mapping of detector UIDs assigned by remage to the Geant4 ntuple
     * IDs and the ntuple names (written to disk).
     *
     * @details Used for output tables that log stepping data in a certain
     * detector.
     *
     * @code
     *   {uid, {ntuple_id, ntuple_name}}
     * @endcode
     */
    static G4ThreadLocal std::map<int, std::pair<int, std::string>> fNtupleIDs;

    /** @brief Mapping of auxiliary ntuple names to the corresponding Geant4
     * ntuple IDs.
     *
     * @details This is used to keep track of "auxiliary" tables that do not
     * refer to stepping data from a detector (see usage in @ref
     * RMGVertexOutputScheme or @c RMGTrackOutputScheme).
     *
     * @code
     *   {ntuple_name, ntuple_id}
     * @endcode
     */
    static G4ThreadLocal std::map<std::string, int> fNtupleAuxIDs;

    static RMGOutputManager* fRMGOutputManager;

    // messenger stuff
    std::unique_ptr<G4GenericMessenger> fOutputMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
