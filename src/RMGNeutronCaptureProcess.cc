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


#include "RMGNeutronCaptureProcess.hh"

#include "G4Element.hh"
#include "G4Gamma.hh"
#include "G4IonTable.hh"
#include "G4Isotope.hh"
#include "G4Neutron.hh"
#include "G4NeutronCaptureXS.hh"
#include "G4NistManager.hh"
#include "G4Nucleus.hh"
#include "G4ParticleChange.hh"
#include "G4ParticleDefinition.hh"
#include "G4RandomDirection.hh"
#include "G4Step.hh"
#include "G4Track.hh"

#include "RMGGrabmayrGCReader.hh"
#include "RMGLog.hh"

namespace u = CLHEP;

RMGNeutronCaptureProcess::RMGNeutronCaptureProcess(const G4String& processName)
    : G4HadronicProcess(processName, fCapture) {
  AddDataSet(new G4NeutronCaptureXS());
}

G4bool RMGNeutronCaptureProcess::IsApplicable(const G4ParticleDefinition& aParticleType) {
  return (&aParticleType == G4Neutron::Neutron());
}

// Do exactly as in G4HadronicProcess, unless the capture happens on certain isotopes
G4VParticleChange* RMGNeutronCaptureProcess::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
  // Get the proposed result from the "normal" process
  G4VParticleChange* ProposedResult = G4HadronicProcess::PostStepDoIt(aTrack, aStep);

  auto CascadeReader = RMGGrabmayrGCReader::GetInstance();

  G4int nSec = ProposedResult->GetNumberOfSecondaries();
  G4int z;
  G4int a;
  G4bool IsApplicable = false;
  // Search through the proposed secondaries for our Isotopes of interest
  if (nSec > 0) {
    for (G4int i = 0; i < nSec; ++i) {
      const auto particle = ProposedResult->GetSecondary(i)->GetParticleDefinition();
      z = particle->GetAtomicNumber();
      a = particle->GetAtomicMass() -
          1; // -1 because this is after the neutron capture, but the functions expect the target nucleus
      if (z == 0 || a == 0) continue; // not an isotope, but any other particle. Quick skip
      if (CascadeReader->IsApplicable(z, a)) {
        IsApplicable = true;
        // As only one Isotope is possible per Capture, break here so z and a remains correct.
        break;
      }
    }
  }
  // If not Applicable we leave everything untouched
  if (!IsApplicable) return ProposedResult;

  // If Applicable we have to propose our own result
  RMGLog::OutDev(RMGLog::debug, "Proposing own neutron capture result");
  // If we do it ourselves, we are responsible that everything is done correctly.
  // Most of this is copied from G4HadronicProcess PostStepDoIt() or FillResult(), as we are replacing that method.
  theTotalResult->Clear();
  theTotalResult->Initialize(aTrack);
  fWeight = aTrack.GetWeight();
  theTotalResult->ProposeWeight(fWeight);

  // Can skip all of the Checks for illegal track status, as in that case no Applicable isotope would have been produced!

  // Do our Physics.
  GammaCascadeLine input = CascadeReader->GetNextEntry(z, a);
  G4ThreeVector location = aTrack.GetPosition();
  G4double time = aTrack.GetGlobalTime();

  // Expect no deposition as the total energy will be distributed
  theTotalResult->ProposeLocalEnergyDeposit(0.0);
  theTotalResult->ProposeTrackStatus(fStopAndKill);
  theTotalResult->ProposeEnergy(0.0);

  // Produce the created Isotope + the Gammas
  theTotalResult->SetNumberOfSecondaries(input.m + 1);
  G4IonTable* theTable = G4IonTable::GetIonTable();

  auto particleDef_nuc = theTable->GetIon(z, a + 1, (double)(input.em * u::keV));
  auto particle_nuc = new G4DynamicParticle(particleDef_nuc, G4RandomDirection());
  auto secondary_nuc = new G4Track(particle_nuc, time, location);

  // secondary_nuc->SetCreatorModelID(idModel); No idea what this should be. Not relevant?
  secondary_nuc->SetWeight(fWeight);
  secondary_nuc->SetTouchableHandle(aTrack.GetTouchableHandle());
  theTotalResult->AddSecondary(secondary_nuc);

  auto particleDef_gamma = G4Gamma::Gamma();
  for (const G4int energy : input.eg) {
    auto particle_gamma =
        new G4DynamicParticle(particleDef_gamma, energy * u::keV, G4RandomDirection());
    auto secondary_gamma = new G4Track(particle_gamma, time, location);
    secondary_gamma->SetWeight(fWeight);
    secondary_gamma->SetTouchableHandle(aTrack.GetTouchableHandle());
    secondary_gamma->SetKineticEnergy(energy * u::keV);
    theTotalResult->AddSecondary(secondary_gamma);
  }

  return theTotalResult;
}
