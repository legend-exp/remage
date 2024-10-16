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


#include "RMGOpWLSProcess.hh"

#include "G4Threading.hh"
#include "Randomize.hh"

#include "RMGLog.hh"

RMGOpWLSProcess::RMGOpWLSProcess(const G4String& aNamePrefix, G4ProcessType aType)
    : G4WrapperProcess(aNamePrefix, aType) {}

G4VParticleChange* RMGOpWLSProcess::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {

  auto particleChange = pRegProcess->PostStepDoIt(aTrack, aStep);

  if (particleChange->GetNumberOfSecondaries() == 0 &&
      particleChange->GetTrackStatus() == fStopAndKill) {
    // having 0 emitted photons is still expected in some minor cases (probably when emission and
    // absorption spectra overlap).
    return particleChange;
  }

  auto mat = aTrack.GetMaterial();

  // ... otherwise we expect to get exactly 1 shifted photon out, and the old track to be stopped.
  if (particleChange->GetNumberOfSecondaries() != 1 ||
      particleChange->GetTrackStatus() != fStopAndKill) {
    RMGLog::OutFormat(RMGLog::error,
        "{}: Got unexpected particleChange with status={} numOfSecondaries={} (in material {})",
        GetProcessName(), particleChange->GetTrackStatus(),
        particleChange->GetNumberOfSecondaries(), mat->GetName());
    return particleChange;
  }

  auto mpt = mat->GetMaterialPropertiesTable();
  // this case can happen when no original WLSMEANNUMBERPHOTONS had been specified. We still expect
  // to get 1 photon as asserted above, but do not have to sample the efficiency.
  if (!mpt || !mpt->ConstPropertyExists("RMG_WLSMEANNUMBERPHOTONS")) return particleChange;

  // actually sample the WLS efficiency, and remove the (single) secondary photon if necessary.
  double mean_num = mpt->GetConstProperty("RMG_WLSMEANNUMBERPHOTONS");
  if (G4UniformRand() > mean_num) particleChange->ProposeTrackStatus(fKillTrackAndSecondaries);

  return particleChange;
}

////////////////////////////////////////////////////////////////////////////////////////////

void RMGOpWLSProcess::BuildPhysicsTable(const G4ParticleDefinition& aParticleType) {

  pRegProcess->BuildPhysicsTable(aParticleType);

  RMGLog::OutFormat(RMGLog::detail, "{}: replace WLS mean number of emitted photons",
      GetProcessName());

  const auto materialTable = G4Material::GetMaterialTable();
  for (auto mat : *materialTable) {
    auto mpt = mat->GetMaterialPropertiesTable();
    if (!mpt || !mpt->ConstPropertyExists(kWLSMEANNUMBERPHOTONS)) continue;

    // replace the original WLS mean number property with a RMG_-prefixed one. There is no other way
    // to stop G4OpWLS from performing poissonian sampling.
    double mean_num = mpt->GetConstProperty(kWLSMEANNUMBERPHOTONS);
    if (mean_num > 1.0) {
      RMGLog::OutFormat(RMGLog::warning,
          "{}: found WLS mean emission number > 1 for material {} - process not applicable!",
          GetProcessName(), mat->GetName());
      continue;
    }

    // just for safety.
    if (!G4Threading::IsMasterThread()) {
      RMGLog::OutFormat(RMGLog::fatal, "{}: trying to modify gemometry from worker thread",
          GetProcessName());
      continue;
    }

    mpt->AddConstProperty("RMG_WLSMEANNUMBERPHOTONS", mean_num, true);
    mpt->RemoveConstProperty("WLSMEANNUMBERPHOTONS");

    RMGLog::OutFormat(RMGLog::debug, "{}: removed original WLS mean emission number for material {}",
        GetProcessName(), mat->GetName());
  }
}
