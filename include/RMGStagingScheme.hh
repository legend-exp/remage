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

#ifndef _RMG_STAGING_SCHEME_HH_
#define _RMG_STAGING_SCHEME_HH_

#include <fstream>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "G4GenericMessenger.hh"
#include "globals.hh"

#include "RMGVOutputScheme.hh"

class G4Step;

// It is important that these are all 4 bytes so we avoid padding.
/** @brief Minimalistic 52 bytes structure to hold the state of an optical photon. */
struct RMGPhotonState {
    float x, y, z;
    float xmom, ymom, zmom;
    float xpol, ypol, zpol;
    float energy;
    float time;
    float weight;
    int parentid;
};

/** @brief Minimalistic 44 bytes structure to hold the state of an electron. */
struct RMGElectronState {
    float x, y, z;
    float xmom, ymom, zmom;
    int pdgcode;
    float energy;
    float time;
    float weight;
    int parentid;
};

// The states are written to / read back from the scratch file as raw bytes, so the read-back
// interpretation depends on these exact layouts. Guard against a field edit silently desyncing them.
static_assert(std::is_trivially_copyable_v<RMGPhotonState> && sizeof(RMGPhotonState) == 52);
static_assert(std::is_trivially_copyable_v<RMGElectronState> && sizeof(RMGElectronState) == 44);

/** @brief Centralized staging policy for waiting-stack based track deferral. */
class RMGStagingScheme : public RMGVOutputScheme {

  public:

    RMGStagingScheme();

    /** @brief Close the scratch-file descriptors kept open for @c ftruncate. Not needed for the
     *  crash-safe cleanup. this only avoids leaking an fd on an ordinary shutdown. */
    ~RMGStagingScheme() override;

    // -- Framework hooks, in the order the run invokes them --

    /** @brief This class has no output names to assign, but needs some initialization at run start,
     *  invoked in @c RMGRunAction::SetupAnalysisManager */
    void AssignOutputNames(G4AnalysisManager*) override;

    /** @brief The particles this scheme claims control over. */
    [[nodiscard]] std::set<int> GetClaimedParticles() const override;

    /** @brief Clear the staging vectors and files before each event. */
    void ClearBeforeEvent() override;

    /** @brief Wraps @c G4UserStackingAction::StackingActionClassify
     *  @details This classifies configured optical photons and electrons as @c fWaiting.
     */
    std::optional<G4ClassificationOfNewTrack> StackingActionClassify(const G4Track*, int) override;

    /** @brief Evaluate optional stepping-time suspension criteria for configured particles. */
    void SteppingAction(const G4Step*) override;

    /** @brief Re-inject the staged optical photons / electrons for a kept event.
     *  @details Called by @c RMGStackingAction::NewStage after stage 0 finished. Rebuilds the
     *  killed tracks (photons keep their polarization) from the in-memory buffers and the scratch
     *  file, and pushes them onto the urgent stack so they get tracked in the following stage. */
    void ReinjectStagedTracks() override;

    /** @brief Delete the temporary staging files at the end of a run. */
    void EndOfRunAction(const G4Run*) override;

    // -- Configuration setters, bound to the macro commands in DefineCommands --

    /** @brief Set the minimum distance to a Germanium detector surface for an electron to be staged.
     *  @details Set to 0 (the default) to stage electrons regardless of surface distance.
     */
    void SetElectronVolumeSafety(double safety) { fElectronVolumeSafety = safety; }

    /** @brief Add a volume name in which electron staging is active. */
    void AddElectronVolumeName(std::string volume) { fElectronVolumeNames.insert(volume); }

    /** @brief Set the maximum kinetic energy for e- tracks to be considered for staging.
     *  @details Only tracks with kinetic energy below this threshold will be staged.
     *  If set to a negative value, no energy threshold is applied.
     */
    void SetElectronMaxEnergyThresholdForStacking(double energy) {
      fElectronMaxEnergyThresholdForStacking = energy;
    }

    /** @brief Set the minimum kinetic energy for e- tracks to be considered for staging.
     *  @details Only tracks with kinetic energy above this threshold will be staged. This is
     *  useful to avoid staging low-energy electrons (e.g. below the Cherenkov threshold) that
     *  cannot produce the optical photons staging is meant to capture.
     *  If set to a negative value, no energy threshold is applied.
     */
    void SetElectronMinEnergyThresholdForStacking(double energy) {
      fElectronMinEnergyThresholdForStacking = energy;
    }

  private:

    // -- Internal helpers --

    void DefineCommands();

    std::optional<G4ClassificationOfNewTrack> Classify_OpticalPhoton(const G4Track* aTrack);
    std::optional<G4ClassificationOfNewTrack> Classify_ElectronLike(const G4Track* aTrack);

    // Create, open and immediately unlink a scratch file, so it cannot outlive the process. Keeps
    // the descriptor in @c fd for the per-event ftruncate.
    void OpenScratchFile(const std::string& dir, std::fstream& stream, int& fd);

    // Truncate a scratch file back to zero bytes (releasing the disk blocks) and rewind it for the
    // next event, resetting the spill/read counters.
    void ResetScratch(std::fstream& stream, int fd, size_t& valid_bytes, size_t& read_bytes);

    template<class T>
    void FlushToTempFile(std::vector<T>& buffer, std::fstream& stream, size_t& valid_bytes);

    // Re-inject one memory-limited batch of staged tracks: the in-memory buffer first (drained in
    // slices, which is what keeps re-injection bounded when no scratch directory was configured),
    // then the spilled records streamed back in chunks of the same size. @c push rebuilds and
    // stacks a single record. Returns true if a batch was pushed, false once this buffer and its
    // spilled region are fully drained.
    template<class State, class PushFn>
    bool ReinjectBatch(
        std::vector<State>& mem,
        std::fstream& scratch,
        size_t& valid_bytes,
        size_t& read_bytes,
        size_t limit_mb,
        PushFn push
    );

    // -- Owned messengers --

    std::unique_ptr<G4GenericMessenger> fOpticalPhotonStagingMessengers;
    std::unique_ptr<G4GenericMessenger> fElectronStagingMessengers;

    // Variables controlled by macros.
    bool fDeferOpticalPhotonsToWaitingStage = false;
    bool fRMGOpticaldeferring = false;
    bool fDeferElectronsToWaitingStage = false;
    bool fDeferPositronsToWaitingStage = false;
    bool fRMGElectrondeferring = false;
    size_t fOpticalSize = 510; // size of the staging vector before spilling in a scratch file, in MB
    size_t fElectronSize = 120; // This only applies if an associated scratch path is specified.
    std::string fOpticalStorePath = ""; // If specified, the staging vector can spill to a scratch file
    std::string fElectronStorePath = ""; // generated in this directory.
    double fElectronMaxEnergyThresholdForStacking = -1;
    double fElectronMinEnergyThresholdForStacking = -1;
    bool fSuspendElectronsOnEnergyDrop = false;
    double fElectronVolumeSafety = 0;
    std::set<std::string> fElectronVolumeNames;

    // The scratch files are opened read/write and immediately unlinked, so they have no name on
    // disk and the kernel reclaims their space whenever this process dies, however it dies.
    std::fstream fPhotonScratch;
    std::fstream fElectronScratch;
    // The mkstemp fd is kept open (in addition to the fstream's own) so the now-unlinked file can
    // be ftruncate()d back to zero every event.
    int fPhotonScratchFd = -1;
    int fElectronScratchFd = -1;
    size_t fPhotonScratchBytes = 0; // bytes spilled this event
    size_t fPhotonScratchRead = 0;  // bytes of those already consumed by re-injection
    size_t fElectronScratchBytes = 0;
    size_t fElectronScratchRead = 0;

    // When spilling is enabled the record buffers are reserved to exactly these element counts
    // (= LimitMemory) and flushed the moment they are full, so they never reallocate: capacity is
    // pinned at the limit for the whole run and cannot overshoot via std::vector's growth factor.
    // Left at 0 when no scratch file is configured, in which case the buffer grows uncapped.
    size_t fPhotonFlushCount = 0;
    size_t fElectronFlushCount = 0;
    std::vector<RMGPhotonState> fPhotonStates;
    std::vector<RMGElectronState> fElectronStates;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
