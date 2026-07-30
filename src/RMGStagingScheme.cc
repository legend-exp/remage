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

#include "RMGStagingScheme.hh"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <unistd.h>

#include "G4DynamicParticle.hh"
#include "G4Electron.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4Positron.hh"
#include "G4StackManager.hh"
#include "G4Step.hh"
#include "G4Threading.hh"
#include "G4Track.hh"

#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGOutputManager.hh"
#include "RMGOutputTools.hh"

// Warn below this much free space in a staging directory: staged tracks routinely need several GB
static constexpr std::uintmax_t kMinScratchSpace = 10ULL * 1024 * 1024 * 1024;

RMGStagingScheme::RMGStagingScheme() { this->DefineCommands(); }

RMGStagingScheme::~RMGStagingScheme() {
  // The fstreams close their own descriptors; close the extra ones kept for ftruncate.
  if (fPhotonScratchFd != -1) close(fPhotonScratchFd);
  if (fElectronScratchFd != -1) close(fElectronScratchFd);
}

void RMGStagingScheme::AssignOutputNames(G4AnalysisManager*) {
  // Staging resets its per-event buffers in ClearBeforeEvent, which the framework only calls when
  // output persistency is enabled.
  const bool staging_active = fDeferOpticalPhotonsToWaitingStage || fDeferElectronsToWaitingStage ||
                              fDeferPositronsToWaitingStage;
  // We test HasOutputFileName() because that is what actually drives persistency, and it is
  // the only signal available here: IsPersistencyEnabled() is not flipped off until after this
  // function returns.
  if (staging_active && !RMGOutputManager::Instance()->HasOutputFileName())
    RMGLog::Out(
        RMGLog::fatal,
        "The Staging output scheme requires output persistency, but it is disabled. Enable it by "
        "setting an output file (remage -o <file> or /RMG/Output/FileName)."
    );

  const bool photons_spill = fDeferOpticalPhotonsToWaitingStage && fRMGOpticaldeferring &&
                             !fOpticalStorePath.empty();
  // Positrons are staged independently of the electron flag, so either one activates the buffer.
  const bool electrons_spill = (fDeferElectronsToWaitingStage || fDeferPositronsToWaitingStage) &&
                               fRMGElectrondeferring && !fElectronStorePath.empty();

  // Warn about a nearly full scratch directory.
  if (G4Threading::IsMasterThread()) {
    std::set<std::string> dirs;
    if (photons_spill) dirs.insert(fOpticalStorePath);
    if (electrons_spill) dirs.insert(fElectronStorePath);
    for (const auto& dir : dirs) {
      std::error_code ec;
      const auto space = std::filesystem::space(dir, ec);
      // Do not report here if the directory is unusable, OpenScratchFile gives a better message.
      if (ec) continue;
      if (space.available < kMinScratchSpace)
        RMGLog::Out(
            RMGLog::warning,
            "Only ",
            space.available / (1024 * 1024 * 1024),
            " GB available in staging directory '",
            dir,
            "'. Staged tracks can need several GB per thread and per event; if the filesystem runs "
            "full the run aborts."
        );
    }
  }

  // In multithreaded mode the master thread processes no events, so it needs no scratch file.
  if (G4Threading::IsMasterThread() && !RMGManager::Instance()->IsExecSequential()) return;

  // Only when spilling is enabled: reserve each buffer to exactly its spill threshold. Combined with
  // flushing when the buffer is full (see the record sites), the vectors never reallocate and their
  // capacity stays pinned at the limit for the whole run - re-injection batches are much smaller and
  // fit inside comfortably. Without a scratch file the buffers are left to grow uncapped, as before.
  if (photons_spill) {
    OpenScratchFile(fOpticalStorePath, fPhotonScratch, fPhotonScratchFd);
    fPhotonFlushCount = fOpticalSize * 1024 * 1024 / sizeof(RMGPhotonState);
    fPhotonStates.reserve(fPhotonFlushCount);
  }
  if (electrons_spill) {
    OpenScratchFile(fElectronStorePath, fElectronScratch, fElectronScratchFd);
    fElectronFlushCount = fElectronSize * 1024 * 1024 / sizeof(RMGElectronState);
    fElectronStates.reserve(fElectronFlushCount);
  }
}

std::set<int> RMGStagingScheme::GetClaimedParticles() const {
  // Declared regardless of fRMG*deferring: on either path this scheme decides the fate of these
  // particles, so no other scheme may classify them as well.
  std::set<int> claimed;
  if (fDeferOpticalPhotonsToWaitingStage)
    claimed.insert(G4OpticalPhoton::OpticalPhotonDefinition()->GetPDGEncoding());
  if (fDeferElectronsToWaitingStage) claimed.insert(G4Electron::Definition()->GetPDGEncoding());
  if (fDeferPositronsToWaitingStage) claimed.insert(G4Positron::Definition()->GetPDGEncoding());
  return claimed;
}

void RMGStagingScheme::ClearBeforeEvent() {
  fPhotonStates.clear();
  fElectronStates.clear();
  // Truncate and rewind the scratch files so the previous event's disk blocks are released.
  ResetScratch(fPhotonScratch, fPhotonScratchFd, fPhotonScratchBytes, fPhotonScratchRead);
  ResetScratch(fElectronScratch, fElectronScratchFd, fElectronScratchBytes, fElectronScratchRead);
}

std::optional<G4ClassificationOfNewTrack> RMGStagingScheme::StackingActionClassify(
    const G4Track* aTrack,
    int stage
) {
  if (stage != 0) return std::nullopt; // only apply staging logic in stage 0

  if (aTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    return Classify_OpticalPhoton(aTrack);
  } else if (aTrack->GetDefinition() == G4Electron::Definition()) {
    if (!fDeferElectronsToWaitingStage) return std::nullopt;
    return Classify_ElectronLike(aTrack);
  } else if (aTrack->GetDefinition() == G4Positron::Definition()) {
    // positrons follow the exact same staging conditions as electrons.
    if (!fDeferPositronsToWaitingStage) return std::nullopt;
    return Classify_ElectronLike(aTrack);
  }
  return std::nullopt;
}

void RMGStagingScheme::SteppingAction(const G4Step* step) {

  auto track = step->GetTrack();
  if (track == nullptr) return;

  // Keep primaries unaffected, matching existing staging behavior.
  if (track->GetParentID() == 0) return;

  if (track->GetTrackStatus() != fAlive) return;

  // positrons follow the same energy-drop suspension as electrons, but only when explicitly enabled.
  const auto* def = track->GetDefinition();
  const bool is_electron = def == G4Electron::Definition();
  const bool is_positron = def == G4Positron::Definition() && fDeferPositronsToWaitingStage;
  if (!is_electron && !is_positron) return;

  const bool suspend_on_drop = fSuspendElectronsOnEnergyDrop;
  const double threshold = fElectronMaxEnergyThresholdForStacking;

  if (!suspend_on_drop || threshold < 0) return;

  const auto* pre_step = step->GetPreStepPoint();
  const auto* post_step = step->GetPostStepPoint();
  if (pre_step == nullptr || post_step == nullptr) return;

  const auto pre_energy = pre_step->GetKineticEnergy();
  const auto post_energy = post_step->GetKineticEnergy();

  // Suspend exactly at threshold crossing to avoid repeatedly suspending tracks that always stay
  // below threshold.
  if (pre_energy > threshold && post_energy <= threshold) track->SetTrackStatus(fSuspend);
  // No RMGStacking here, to not mess with particles in play
}

void RMGStagingScheme::ReinjectStagedTracks() {
  // Called by RMGStackingAction::NewStage for kept events, after the stage counter advanced
  auto* stack_man = G4EventManager::GetEventManager()->GetStackManager();
  if (stack_man == nullptr)
    RMGLog::OutDev(RMGLog::fatal, "No G4StackManager available for reinjection of staged tracks");

  size_t skipped = 0;

  auto push_photon = [&](const RMGPhotonState& s) {
    const G4ThreeVector dir(s.xmom, s.ymom, s.zmom);
    // Guard against degenerate records: Geant4 cannot track a photon without energy or direction.
    if (!(s.energy > 0) || !(dir.mag2() > 0)) {
      skipped++;
      return;
    }
    // The float32 round-trip leaves the direction slightly off-unit, and Geant4 assumes unit length.
    auto* dp = new G4DynamicParticle(G4OpticalPhoton::OpticalPhotonDefinition(), dir.unit(), s.energy);
    const G4ThreeVector pol(s.xpol, s.ypol, s.zpol);
    if (pol.mag2() > 0) dp->SetPolarization(pol.unit());
    auto* track = new G4Track(dp, s.time, G4ThreeVector(s.x, s.y, s.z));
    track->SetWeight(s.weight);
    track->SetParentID(s.parentid);
    stack_man->PushOneTrack(track);
  };

  const int electron_pdg = G4Electron::Definition()->GetPDGEncoding();
  const int positron_pdg = G4Positron::Definition()->GetPDGEncoding();
  auto push_electron = [&](const RMGElectronState& s) {
    const G4ThreeVector dir(s.xmom, s.ymom, s.zmom);
    if (!(s.energy >= 0) || !(dir.mag2() > 0)) {
      skipped++;
      return;
    }
    const G4ParticleDefinition* def = nullptr;
    if (s.pdgcode == electron_pdg) def = G4Electron::Definition();
    else if (s.pdgcode == positron_pdg) def = G4Positron::Definition();
    else {
      skipped++;
      return;
    }
    auto* dp = new G4DynamicParticle(def, dir.unit(), s.energy);
    auto* track = new G4Track(dp, s.time, G4ThreeVector(s.x, s.y, s.z));
    track->SetWeight(s.weight);
    track->SetParentID(s.parentid);
    stack_man->PushOneTrack(track);
  };

  // One batch per NewStage call: drain the photon buffer first, the electron buffer afterwards.
  if (!ReinjectBatch(
          fPhotonStates,
          fPhotonScratch,
          fPhotonScratchBytes,
          fPhotonScratchRead,
          fOpticalSize,
          push_photon
      ))
    ReinjectBatch(
        fElectronStates,
        fElectronScratch,
        fElectronScratchBytes,
        fElectronScratchRead,
        fElectronSize,
        push_electron
    );

  if (skipped > 0)
    RMGLog::Out(RMGLog::fatal, "Found ", skipped, " malformed staged track(s) on re-injection");
}

void RMGStagingScheme::EndOfRunAction(const G4Run*) {
  // Nothing to clean up on disk: the scratch files were unlinked as soon as they were opened.
  // A second /run/beamOn in the same session reuses them - AssignOutputNames only runs for the
  // first run and could not create new ones.
  fPhotonStates.clear();
  fElectronStates.clear();
  fPhotonScratchBytes = fPhotonScratchRead = 0;
  fElectronScratchBytes = fElectronScratchRead = 0;
}

std::optional<G4ClassificationOfNewTrack> RMGStagingScheme::Classify_OpticalPhoton(
    const G4Track* aTrack
) {
  if (!fDeferOpticalPhotonsToWaitingStage) return std::nullopt;
  // Custom staging disabled: fall back to G4's waiting stack
  if (!fRMGOpticaldeferring) return fWaiting;
  // If custom staging is enabled, we stack ourselves and kill it.
  const auto& pos = aTrack->GetPosition();
  const auto& dir = aTrack->GetMomentumDirection();
  const auto& pol = aTrack->GetPolarization();
  // Geant4 just swallows the exception here.
  try {
    fPhotonStates.emplace_back(
        static_cast<float>(pos.getX()),
        static_cast<float>(pos.getY()),
        static_cast<float>(pos.getZ()),
        static_cast<float>(dir.getX()),
        static_cast<float>(dir.getY()),
        static_cast<float>(dir.getZ()),
        static_cast<float>(pol.getX()),
        static_cast<float>(pol.getY()),
        static_cast<float>(pol.getZ()),
        static_cast<float>(aTrack->GetKineticEnergy()),
        static_cast<float>(aTrack->GetGlobalTime()),
        static_cast<float>(aTrack->GetWeight()),
        aTrack->GetParentID()
    );
  } catch (const std::exception& e) {
    RMGLog::OutDev(RMGLog::fatal, "Error while emplacing optical photon state: ", e.what());
  }
  // Flush once the buffer is full, before the next emplace_back could reallocate.
  // If no scratch file the buffer grows uncapped.
  if (fPhotonScratch.is_open() && fPhotonStates.size() >= fPhotonFlushCount) {
    FlushToTempFile(fPhotonStates, fPhotonScratch, fPhotonScratchBytes);
  }
  return fKill;
}

std::optional<G4ClassificationOfNewTrack> RMGStagingScheme::Classify_ElectronLike(
    const G4Track* aTrack
) {
  // do not touch the primary track of an event.
  if (aTrack->GetParentID() == 0) return std::nullopt;

  // if a max energy threshold is set, only defer tracks below that threshold.
  if (fElectronMaxEnergyThresholdForStacking >= 0 &&
      aTrack->GetKineticEnergy() > fElectronMaxEnergyThresholdForStacking)
    return std::nullopt;

  // if a min energy threshold is set, only defer tracks above that threshold.
  if (fElectronMinEnergyThresholdForStacking >= 0 &&
      aTrack->GetKineticEnergy() < fElectronMinEnergyThresholdForStacking)
    return std::nullopt;

  const auto* volume = aTrack->GetVolume();
  if (volume == nullptr) return std::nullopt;

  // If volume names are configured, only apply electron staging inside those volumes.
  // (If none are configured, electron staging applies to all volumes.)
  if (!fElectronVolumeNames.empty()) {
    const auto vol_name = volume->GetLogicalVolume()->GetName();
    if (!fElectronVolumeNames.contains(vol_name)) return std::nullopt;
  }

  // stop if electron safety is not configured.
  if (fElectronVolumeSafety < 0) return std::nullopt;

  // if safety is non-zero, only defer tracks that have a minimum distance to Germanium
  // detector surfaces. (A safety of zero always defers, regardless of surface distance.)
  if (fElectronVolumeSafety != 0) {
    bool is_within_safety = RMGOutputTools::is_within_surface_safety(
        volume,
        aTrack->GetPosition(),
        fElectronVolumeSafety,
        /*is_distance_check_germanium_only=*/true
    );
    if (is_within_safety) return std::nullopt;
  }

  // this electron is staged. Either use G4's stack or RMG's custom staging
  if (!fRMGElectrondeferring) return fWaiting;

  // use custom staging
  const auto& pos = aTrack->GetPosition();
  const auto& dir = aTrack->GetMomentumDirection();
  const auto energy = aTrack->GetKineticEnergy();
  const auto time = aTrack->GetGlobalTime();
  const int pdgcode = aTrack->GetDefinition()->GetPDGEncoding();
  // Geant4 just swallows the exception here.
  try {
    fElectronStates.emplace_back(
        static_cast<float>(pos.getX()),
        static_cast<float>(pos.getY()),
        static_cast<float>(pos.getZ()),
        static_cast<float>(dir.getX()),
        static_cast<float>(dir.getY()),
        static_cast<float>(dir.getZ()),
        pdgcode,
        static_cast<float>(energy),
        static_cast<float>(time),
        static_cast<float>(aTrack->GetWeight()),
        aTrack->GetParentID()
    );
  } catch (const std::exception& e) {
    RMGLog::OutDev(RMGLog::fatal, "Error while emplacing electron state: ", e.what());
  }
  // Flush once the buffer is full, before the next emplace_back
  // could reallocate. See Classify_OpticalPhoton for the rationale and the keep-in-memory path.
  if (fElectronScratch.is_open() && fElectronStates.size() >= fElectronFlushCount) {
    FlushToTempFile(fElectronStates, fElectronScratch, fElectronScratchBytes);
  }
  return fKill;
}

// Create a unique scratch file in dir, open it read/write and immediately unlink it. The open
// handle keeps the inode alive, so the file has no name on disk and the kernel reclaims its space
// whenever this process dies
void RMGStagingScheme::OpenScratchFile(const std::string& dir, std::fstream& stream, int& fd) {
  std::string pattern = (std::filesystem::path(dir) / "rmg_XXXXXX").string();
  std::vector<char> buffer(pattern.begin(), pattern.end());
  buffer.push_back('\0');

  // This also covers a missing / non-writable staging directory (ENOENT, EACCES). The fd is kept
  // open so we can ftruncate the file each event; the fstream reopens it by name for the actual I/O.
  fd = mkstemp(buffer.data());
  if (fd == -1)
    RMGLog::Out(
        RMGLog::fatal,
        "Could not create a staging scratch file in '",
        dir,
        "': ",
        std::strerror(errno)
    );

  const std::filesystem::path path(buffer.data());
  stream.open(path, std::ios::in | std::ios::out | std::ios::binary);
  if (!stream.is_open())
    RMGLog::Out(RMGLog::fatal, "Could not open staging scratch file '", path.string(), "'");

  // Only badbit: a real I/O error (e.g. the disk running full) throws and is caught where we write,
  // while a short read at the end of the spilled region sets failbit and has to stay harmless.
  stream.exceptions(std::ios::badbit);

  std::error_code ec;
  std::filesystem::remove(path, ec);
  if (ec)
    RMGLog::Out(
        RMGLog::warning,
        "Could not unlink staging scratch file '",
        path.string(),
        "'; it may be left behind if this process is killed: ",
        ec.message()
    );
}

void RMGStagingScheme::ResetScratch(std::fstream& stream, int fd, size_t& valid_bytes, size_t& read_bytes) {
  if (!stream.is_open()) return;
  valid_bytes = read_bytes = 0;
  // Release the previous event's blocks, not merely rewind:
  if (fd != -1 && ftruncate(fd, 0) != 0)
    RMGLog::Out(
        RMGLog::fatal,
        "Could not truncate staging scratch file (aborting to avoid filling the disk): ",
        std::strerror(errno)
    );
  stream.clear(); // re-injection leaves eofbit set, which would make the next write fail
  stream.seekp(0);
}

template<class T>
inline void RMGStagingScheme::FlushToTempFile(
    std::vector<T>& buffer,
    std::fstream& stream,
    size_t& valid_bytes
) {
  if (buffer.empty()) return;
  const size_t n_bytes = buffer.size() * sizeof(T);
  // Geant4 just swallows exceptions, so catch it ourselves.
  try {
    stream.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(n_bytes));
  } catch (const std::exception& e) {
    RMGLog::OutDev(RMGLog::fatal, "Error while flushing to scratch file. Likely full disk: ", e.what());
  }
  valid_bytes += n_bytes;
  buffer.clear();
}

template<class State, class PushFn>
bool RMGStagingScheme::ReinjectBatch(
    std::vector<State>& mem,
    std::fstream& scratch,
    size_t& valid_bytes,
    size_t& read_bytes,
    size_t limit_mb,
    PushFn push
) {
  // The limit is about how much memory the staged tracks may occupy, so size the batch by what the
  // rebuilt tracks cost on the Geant4 stacks. Taken from the actual types, so this follows the Geant4 version in use.
  constexpr size_t heap_overhead = 16; // approximate per-allocation malloc header/alignment
  constexpr size_t track_footprint = sizeof(G4Track) + sizeof(G4DynamicParticle) +
                                     sizeof(G4StackedTrack) + 2 * heap_overhead;
  // Clamp the limit: a zero would degenerate into one track - and thus one stage - per record.
  const size_t batch = std::max<size_t>(
      1,
      (std::max<size_t>(limit_mb, 1) * 1024 * 1024) / track_footprint
  );

  // Phase A: drain the in-memory buffer in batches. Taking them off the back is O(1)
  if (!mem.empty()) {
    const size_t n = std::min(batch, mem.size());
    for (size_t i = mem.size() - n; i < mem.size(); ++i) push(mem[i]);
    mem.resize(mem.size() - n);
    return true;
  }

  // Phase B: stream the spilled records back, one batch per stage. The valid region is delimited by
  // the counters, so there is no end-of-file to discover and the handle is never reopened.
  if (!scratch.is_open() || read_bytes >= valid_bytes) return false;

  const size_t n = std::min(batch, (valid_bytes - read_bytes) / sizeof(State));
  if (n == 0) return false;

  mem.resize(n);
  try {
    // Seeking is also what allows the stream to switch from writing to reading, and it flushes the
    // pending output on the way. Geant4 would swallow any exception thrown here.
    scratch.seekg(static_cast<std::streamoff>(read_bytes));
    scratch.read(reinterpret_cast<char*>(mem.data()), static_cast<std::streamsize>(n * sizeof(State)));
  } catch (const std::exception& e) {
    RMGLog::OutDev(RMGLog::fatal, "Error while reading back the staging scratch file: ", e.what());
  }

  const size_t got = static_cast<size_t>(scratch.gcount()) / sizeof(State);
  read_bytes += got * sizeof(State);
  for (size_t i = 0; i < got; ++i) push(mem[i]);
  mem.clear();
  return got > 0;
}

void RMGStagingScheme::DefineCommands() {

  fOpticalPhotonStagingMessengers = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Staging/OpticalPhotons/",
      "Commands for staging optical photon tracks."
  );

  fOpticalPhotonStagingMessengers
      ->DeclareProperty("DeferToWaitingStage", fDeferOpticalPhotonsToWaitingStage)
      .SetGuidance("Defer optical photons to the waiting stack during stage 0.")
      .SetGuidance(
          std::string("This is ") + (fDeferOpticalPhotonsToWaitingStage ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fOpticalPhotonStagingMessengers->DeclareProperty("RMGdeferring", fRMGOpticaldeferring)
      .SetGuidance("Use the minimalistic remage custom staging, which reduces memory usage.")
      .SetGuidance(
          std::string("This is ") + (fRMGOpticaldeferring ? "enabled" : "disabled") + " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fOpticalPhotonStagingMessengers->DeclareProperty("LimitMemory", fOpticalSize)
      .SetGuidance("Set the allowed size of the staging vector (for each thread), before flushed to a temp file, in MB.")
      .SetGuidance(std::string("This is ") + (std::to_string(fOpticalSize)) + " MB by default.")
      .SetParameterName("integer", true)
      .SetDefaultValue("510")
      .SetStates(G4State_Idle);

  fOpticalPhotonStagingMessengers->DeclareProperty("StorePath", fOpticalStorePath)
      .SetGuidance("Set the directory in which the temp files for optical photons are stored.")
      .SetGuidance("If not specified, no temp files are created and memory consumption is uncapped.")
      .SetParameterName("string", true)
      .SetDefaultValue("")
      .SetStates(G4State_Idle);

  fElectronStagingMessengers = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/Staging/Electrons/",
      "Commands for staging electron tracks."
  );

  fElectronStagingMessengers->DeclareProperty("DeferToWaitingStage", fDeferElectronsToWaitingStage)
      .SetGuidance("Defer secondary electrons to the waiting stack during stage 0.")
      .SetGuidance(
          std::string("This is ") + (fDeferElectronsToWaitingStage ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fElectronStagingMessengers->DeclareProperty("IncludePositrons", fDeferPositronsToWaitingStage)
      .SetGuidance("Also defer secondary positrons to the waiting stack during stage 0.")
      .SetGuidance(
          "Positrons are subject to the same staging conditions as electrons "
          "(energy thresholds, volume safety and volume names)."
      )
      .SetGuidance(
          std::string("This is ") + (fDeferPositronsToWaitingStage ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fElectronStagingMessengers
      ->DeclareMethodWithUnit("VolumeSafety", "cm", &RMGStagingScheme::SetElectronVolumeSafety)
      .SetGuidance(
          "Set the minimum distance to a Germanium detector surface for this electron to be staged."
      )
      .SetGuidance("Set to 0 (the default) to stage regardless of surface distance.")
      .SetParameterName("safety", false)
      .SetStates(G4State_Idle);

  fElectronStagingMessengers
      ->DeclareMethod("AddVolumeName", &RMGStagingScheme::AddElectronVolumeName)
      .SetGuidance("Add a volume name in which electron staging is active.")
      .SetGuidance("If this command is not called, electron staging applies to all volumes.")
      .SetParameterName("volume", false)
      .SetStates(G4State_Idle);

  fElectronStagingMessengers
      ->DeclareMethodWithUnit(
          "MaxEnergyThresholdForStacking",
          "MeV",
          &RMGStagingScheme::SetElectronMaxEnergyThresholdForStacking
      )
      .SetGuidance("Set the maximum kinetic energy for e- tracks to be considered for staging.")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fElectronStagingMessengers
      ->DeclareMethodWithUnit(
          "MinEnergyThresholdForStacking",
          "MeV",
          &RMGStagingScheme::SetElectronMinEnergyThresholdForStacking
      )
      .SetGuidance("Set the minimum kinetic energy for e- tracks to be considered for staging.")
      .SetGuidance("Useful to skip staging low-energy electrons (e.g. below the Cherenkov threshold).")
      .SetParameterName("threshold", false)
      .SetStates(G4State_Idle);

  fElectronStagingMessengers->DeclareProperty("SuspendOnEnergyDrop", fSuspendElectronsOnEnergyDrop)
      .SetGuidance("Suspend secondary electrons when they cross from above to below the configured kinetic-energy threshold.")
      .SetGuidance("The threshold is taken from MaxEnergyThresholdForStacking.")
      .SetGuidance(
          std::string("This is ") + (fSuspendElectronsOnEnergyDrop ? "enabled" : "disabled") +
          " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("false")
      .SetStates(G4State_Idle);

  fElectronStagingMessengers->DeclareProperty("RMGdeferring", fRMGElectrondeferring)
      .SetGuidance(
          "Use the minimalistic remage custom staging for electrons, which reduces memory usage."
      )
      .SetGuidance("When disabled, staged electrons are deferred to the waiting stack instead.")
      .SetGuidance(
          std::string("This is ") + (fRMGElectrondeferring ? "enabled" : "disabled") + " by default."
      )
      .SetParameterName("boolean", true)
      .SetDefaultValue("true")
      .SetStates(G4State_Idle);

  fElectronStagingMessengers->DeclareProperty("LimitMemory", fElectronSize)
      .SetGuidance("Set the allowed size of the electron staging vector (for each thread), before flushed to a temp file, in MB.")
      .SetGuidance(std::string("This is ") + (std::to_string(fElectronSize)) + " MB by default.")
      .SetParameterName("integer", true)
      .SetDefaultValue("120")
      .SetStates(G4State_Idle);

  fElectronStagingMessengers->DeclareProperty("StorePath", fElectronStorePath)
      .SetGuidance("Set the directory in which the temp files for staged electrons are stored.")
      .SetGuidance("If not specified, no temp files are created and the staging vector ignores the memory limit.")
      .SetParameterName("string", true)
      .SetDefaultValue("")
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab
