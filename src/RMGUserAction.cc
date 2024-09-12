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

#include "RMGUserAction.hh"

#include <memory>

#include "G4MultiSteppingAction.hh"
#include "G4MultiTrackingAction.hh"

#include "RMGEventAction.hh"
#include "RMGGrabmayrGCReader.hh"
#include "RMGManager.hh"
#include "RMGMasterGenerator.hh"
#include "RMGRunAction.hh"
#include "RMGStackingAction.hh"
#include "RMGSteppingAction.hh"
#include "RMGTrackingAction.hh"

void RMGUserAction::BuildForMaster() const {
  // the master thread does not simulate anything.
  // initialize the master generator also on the master thread, to make sure that particle source
  // commands are available early on (following a note in G4GeneralParticleSourceMessenger.hh).
  auto generator_primary = new RMGMasterGenerator();
  this->SetUserAction(
      new RMGRunAction(generator_primary, RMGManager::Instance()->IsPersistencyEnabled()));
  RMGGrabmayrGCReader::GetInstance();
}

void RMGUserAction::Build() const {

  auto generator_primary = new RMGMasterGenerator();
  auto run_action =
      new RMGRunAction(generator_primary, RMGManager::Instance()->IsPersistencyEnabled());
  auto event_action = new RMGEventAction(run_action);

  // Add the remage-internal stepping action and optional user-specified custom stepping actions.
  const auto user_stepping_actions = RMGManager::Instance()->GetUserInit()->GetSteppingActions();
  G4UserSteppingAction* stepping_action = new RMGSteppingAction(event_action);
  if (!user_stepping_actions.empty()) {
    auto multi_stepping_action = new G4MultiSteppingAction();
    multi_stepping_action->push_back(std::unique_ptr<G4UserSteppingAction>(stepping_action));
    for (auto const& step : user_stepping_actions) { multi_stepping_action->push_back(step()); }
    stepping_action = multi_stepping_action;
  }

  // Add the remage-internal tracking action and optional user-specified custom stepping actions.
  const auto user_tracking_actions = RMGManager::Instance()->GetUserInit()->GetTrackingActions();
  G4UserTrackingAction* tracking_action = new RMGTrackingAction(run_action);
  if (!user_tracking_actions.empty()) {
    auto multi_tracking_action = new G4MultiTrackingAction();
    multi_tracking_action->push_back(std::unique_ptr<G4UserTrackingAction>(tracking_action));
    for (auto const& step : user_tracking_actions) { multi_tracking_action->push_back(step()); }
    tracking_action = multi_tracking_action;
  }

  this->SetUserAction(generator_primary);
  this->SetUserAction(event_action);
  this->SetUserAction(run_action);
  this->SetUserAction(new RMGStackingAction(run_action));
  this->SetUserAction(stepping_action);
  this->SetUserAction(tracking_action);
  RMGGrabmayrGCReader::GetInstance();
}

// vim: tabstop=2 shiftwidth=2 expandtab
