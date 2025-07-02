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

#ifndef _RMG_MANAGER_HH_
#define _RMG_MANAGER_HH_

#include <atomic>
#include <memory>
#include <set>
#include <vector>

#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4Threading.hh"
#include "G4VisManager.hh"
#include "globals.hh"

#include "RMGExceptionHandler.hh"
#include "RMGLog.hh"
#include "RMGOutputManager.hh"
#include "RMGUserInit.hh"

class G4VUserPhysicsList;
class RMGHardware;
class RMGUserAction;
class G4GenericMessenger;
class G4UIExecutive;
/**
 * @brief Main manager class for the remage simulation.
 *
 * This singleton class initializes and manages the Geant4 run manager, visualization,
 * detector construction, physics list, and user actions. It also handles global
 * configurations such as random engine settings and logging.
 */
class RMGManager {

  public:

    RMGManager() = delete;
    /**
     * @brief Constructs a new RMGManager object.
     * @details This constructor has to be called by the user, but can only be called once.
     * @param app_name The application name.
     * @param argc Argument count.
     * @param argv Argument vector.
     */
    RMGManager(std::string app_name, int argc, char** argv);
    /**
     * @brief Default destructor.
     */
    ~RMGManager() = default;

    RMGManager(RMGManager const&) = delete;
    RMGManager& operator=(RMGManager const&) = delete;
    RMGManager(RMGManager&&) = delete;
    RMGManager& operator=(RMGManager&&) = delete;

    // getters
    /**
     * @brief Retrieves the singleton instance of RMGManager.
     * @return Pointer to the RMGManager instance.
     */
    static RMGManager* Instance() { return fRMGManager; }
    /**
     * @brief Retrieves the Geant4 run manager.
     * @return Pointer to the G4RunManager instance.
     */
    G4RunManager* GetG4RunManager();
    /**
     * @brief Retrieves the Geant4 visualization manager.
     * @return Pointer to the G4VisManager instance.
     */
    G4VisManager* GetG4VisManager();
    /**
     * @brief Retrieves the detector construction.
     * @return Pointer to the RMGHardware instance.
     */
    RMGHardware* GetDetectorConstruction();
    /**
     * @brief Retrieves the physics list.
     * @return Pointer to the G4VUserPhysicsList instance.
     */
    G4VUserPhysicsList* GetProcessesList();
    /**
     * @brief Retrieves the user initialization object.
     * @return Shared pointer to RMGUserInit.
     */
    [[nodiscard]] auto GetUserInit() const { return fUserInit; }
    /**
     * @brief Retrieves the output manager.
     * @return Pointer to the RMGOutputManager instance.
     */
    [[nodiscard]] auto GetOutputManager() const { return RMGOutputManager::Instance(); }
    /**
     * @brief Returns the print modulo value.
     * @return Print modulo integer.
     */
    [[nodiscard]] int GetPrintModulo() const { return fPrintModulo; }

    /**
     * @brief Checks if the execution is sequential (single-threaded).
     * @return True if the run manager is sequential.
     */
    [[nodiscard]] bool IsExecSequential() {
      return fG4RunManager->GetRunManagerType() == G4RunManager::RMType::sequentialRM;
    }

    // setters
    /**
     * @brief Sets the Geant4 run manager.
     * @param g4_manager Pointer to the G4RunManager.
     */
    void SetUserInit(G4RunManager* g4_manager) {
      fG4RunManager = std::unique_ptr<G4RunManager>(g4_manager);
    }
    /**
     * @brief Sets the Geant4 visualization manager.
     * @param vis Pointer to the G4VisManager.
     */
    void SetUserInit(G4VisManager* vis) { fG4VisManager = std::unique_ptr<G4VisManager>(vis); }
    /**
     * @brief Sets the detector construction.
     * @param det Pointer to RMGHardware.
     */
    void SetUserInit(RMGHardware* det) { fDetectorConstruction = det; }
    /**
     * @brief Sets the physics list.
     * @param proc Pointer to G4VUserPhysicsList.
     */
    void SetUserInit(G4VUserPhysicsList* proc) { fPhysicsList = proc; }

    /**
     * @brief Enables or disables interactive mode.
     * @param flag True to enable interactive mode.
     */
    void SetInteractive(bool flag = true) { fInteractive = flag; }
    /**
     * @brief Sets the number of threads.
     * @param nthreads Number of threads.
     */
    void SetNumberOfThreads(int nthreads) { fNThreads = nthreads; }
    /**
     * @brief Sets the print modulo value.
     * @param n_ev Number of events for modulo printing.
     */
    void SetPrintModulo(int n_ev) { fPrintModulo = n_ev > 0 ? n_ev : -1; }

    /**
     * @brief Includes a macro or single macro command file for execution.
     * @param filename The name of the macro file.
     */
    void IncludeMacroFile(std::string filename) { fMacroFilesOrContents.emplace_back(filename); }
    /**
     * @brief Registers a Geant4 alias for use in macro commands.
     * @param alias The alias name.
     * @param value The corresponding value.
     */
    void RegisterG4Alias(std::string alias, std::string value) { fG4Aliases.emplace(alias, value); }
    /**
     * @brief Initialize the simulation components (run manager, visualization,
     * random engine, detector construction, physics list, ...).
     *
     * @details This does not call@c Initialize() of \ref G4RunManager which
     * remains the user's responsibility (i.e., by using the macro command @c
     * /run/initialize)
     */
    void Initialize();
    /**
     * @brief Executes the supplied macro files and commands and switch to interactive session if
     * requested.
     * @details This does not actually start the simulation runs; that has to be done with macro
     * commands or by calling int \ref G4RunManager.
     */
    void Run();

    /**
     * @brief Sets the random engine by name.
     * @param name Name of the random engine.
     */
    void SetRandEngine(std::string name);
    /**
     * @brief Sets the seed for the random engine.
     * @param seed The seed value.
     */
    void SetRandEngineSeed(int seed);
    /**
     * @brief Sets the internal seed index for the random engine.
     * @param index The seed index.
     */
    void SetRandEngineInternalSeed(int index);
    /**
     * @brief Sets the random engine seed using system entropy.
     */
    void SetRandSystemEntropySeed();
    /**
     * @brief Applies the random engine settings for the current thread.
     * @return True if applied successfully.
     */
    bool ApplyRandEngineForCurrentThread();
    /**
     * @brief Checks if the random engine is under user controlled seeding.
     * @return True if controlled.
     */
    [[nodiscard]] bool GetRandIsControlled() const { return fIsRandControlled; }
    /**
     * @brief Checks if a random engine has been selected by the user.
     * @return True if a random engine is selected.
     */
    [[nodiscard]] bool GetRandEngineSelected() const { return !fRandEngineName.empty(); }

    /**
     * @brief Sets the logging level.
     * @param level Logging level as a string.
     */
    void SetLogLevel(std::string level);

    /**
     * @brief Checks if any warnings have been recorded.
     * @return True if warnings occurred.
     */
    [[nodiscard]] bool HadWarning() const {
      return fExceptionHandler->HadWarning() || RMGLog::HadWarning();
    }
    /**
     * @brief Checks if any errors have been recorded.
     * @return True if errors occurred.
     */
    [[nodiscard]] bool HadError() const {
      return fExceptionHandler->HadError() || RMGLog::HadError();
    }

    /**
     * @brief Activates an optional output scheme.
     * @param name The name of the optional output scheme to activate.
     */
    // NOLINTNEXTLINE(readability-make-member-function-const)
    void ActivateOptionalOutputScheme(std::string name) {
      GetUserInit()->ActivateOptionalOutputScheme(name);
    }

    /**
     * @brief Set a flag to gracefully aborts the simulation run at the next possible time.
     */
    static void AbortRunGracefully() {
      if (!fAbortRun.is_lock_free()) return;
      fAbortRun = true;
    }
    /**
     * @brief Checks whether an abort signal has been triggered.
     * @return True if the run should be aborted.
     */
    static bool ShouldAbortRun() { return fAbortRun; }

  private:

    void SetUpDefaultG4RunManager(G4RunManagerType type = G4RunManagerType::Default);
    void SetUpDefaultG4VisManager();
    void SetUpDefaultDetectorConstruction();
    void SetUpDefaultProcessesList();
    void SetUpDefaultUserAction();
    void CheckRandEngineMTState();

    std::unique_ptr<G4UIExecutive> StartInteractiveSession();

    std::string fApplicationName;
    int fArgc;
    char** fArgv;
    std::map<std::string, std::string> fG4Aliases;
    std::vector<std::string> fMacroFilesOrContents;
    bool fInteractive = false;
    int fPrintModulo = -1;
    int fNThreads = 1;

    bool fIsRandControlled = false;
    bool fIsRandControlledAtEngineChange = false;
    std::string fRandEngineName;

    static RMGManager* fRMGManager;
    static std::atomic<bool> fAbortRun;

    std::unique_ptr<G4RunManager> fG4RunManager = nullptr;
    std::unique_ptr<G4VisManager> fG4VisManager = nullptr;

    RMGExceptionHandler* fExceptionHandler = nullptr;
    G4VUserPhysicsList* fPhysicsList = nullptr;
    RMGHardware* fDetectorConstruction = nullptr;

    RMGUserAction* fUserAction = nullptr;
    // store partial custom actions to be used later in RMGUserAction
    std::shared_ptr<RMGUserInit> fUserInit;

    // messenger stuff
    std::unique_ptr<G4GenericMessenger> fMessenger;
    std::unique_ptr<G4GenericMessenger> fLogMessenger;
    std::unique_ptr<G4GenericMessenger> fRandMessenger;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
