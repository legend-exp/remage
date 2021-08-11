#include "RMGGeneratorVolumeConfinement.hh"

#include "G4VPhysicalVolume.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4Orb.hh"
#include "G4Box.hh"
#include "G4SubtractionSolid.hh"
#include "G4VisExtent.hh"
#include "G4TransportationManager.hh"
#include "Randomize.hh"
#include "G4GenericMessenger.hh"
#include "G4UnitsTable.hh"

#include "RMGGeneratorUtil.hh"
#include "RMGLog.hh"
#include "RMGNavigationTools.hh"
#include "RMGManager.hh"

#include "RMGTools.hh"

RMGGeneratorVolumeConfinement::SampleableObject::SampleableObject(
  G4VPhysicalVolume* v, G4RotationMatrix r, G4ThreeVector t, G4VSolid* s):

  rotation(r),
  translation(t),
  containment_check(true) {

  if (!v and !s) RMGLog::Out(RMGLog::error, "Invalid pointers given to constructor");

  physical_volume = v;
  sampling_solid = s;

  if (physical_volume) volume = physical_volume->GetLogicalVolume()->GetSolid()->GetCubicVolume();
  else volume = sampling_solid->GetCubicVolume();
}

RMGGeneratorVolumeConfinement::SampleableObject::~SampleableObject() {
  if (sampling_solid and sampling_solid != physical_volume->GetLogicalVolume()->GetSolid()) {
    delete sampling_solid;
  }
}

const RMGGeneratorVolumeConfinement::SampleableObject& RMGGeneratorVolumeConfinement::SampleableObjectCollection::SurfaceWeightedRand() {
  auto choice = total_surface * G4UniformRand();
  G4double w = 0;
  for (const auto& o : data) {
    if (choice > w and choice <= w+o.surface) return o;
    w += o.surface;
    if (w >= total_surface) {
      RMGLog::Out(RMGLog::error, "Sampling from collection of sampleables unespectedly failed ",
          "(out-of-range error). Returning last object");
      return data.back();
    }
  }
  return data.back();
}

const RMGGeneratorVolumeConfinement::SampleableObject& RMGGeneratorVolumeConfinement::SampleableObjectCollection::VolumeWeightedRand() {
  auto choice = total_volume * G4UniformRand();
  G4double w = 0;
  for (const auto& o : data) {
    if (choice > w and choice <= w+o.volume) return o;
    w += o.volume;
    if (w >= total_surface) {
      RMGLog::Out(RMGLog::error, "Sampling from collection of sampleables unespectedly failed ",
          "(out-of-range error). Returning last object");
      return data.back();
    }
  }
  return data.back();
}

G4bool RMGGeneratorVolumeConfinement::SampleableObjectCollection::IsInside(const G4ThreeVector& vertex) {
  auto navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  for (const auto& o : data) {
    if (o.physical_volume) {
      if (navigator->LocateGlobalPointAndSetup(vertex) == o.physical_volume) return true;
    }
    else {
      if (o.sampling_solid->Inside(o.rotation.inverse() * vertex - o.translation)) return true;
    }
  }
  return false;
}

void RMGGeneratorVolumeConfinement::SampleableObjectCollection::emplace_back(G4VPhysicalVolume* v, G4RotationMatrix& r, G4ThreeVector& t, G4VSolid* s) {
  data.emplace_back(v, r, t, s);
  total_volume += data.back().volume;
  total_surface += data.back().surface;
}

void RMGGeneratorVolumeConfinement::SampleableObjectCollection::emplace_back(G4VPhysicalVolume* v, G4RotationMatrix r, G4ThreeVector t, G4VSolid* s) {
  data.emplace_back(v, r, t, s);
  total_volume += data.back().volume;
  total_surface += data.back().surface;
}

/* ========================================================================================== */

RMGGeneratorVolumeConfinement::RMGGeneratorVolumeConfinement() :
  RMGVGeneratorPrimaryPosition("VolumeConfinement"),
  fSamplingMode(SamplingMode::kUnionAll),
  fOnSurface(false),
  fBoundingSolidType("Sphere") {

  this->DefineCommands();
}

void RMGGeneratorVolumeConfinement::InitializePhysicalVolumes() {

  if (!fPhysicalVolumes.empty()) return;

  auto volume_store = G4PhysicalVolumeStore::GetInstance();

  // scan all search patterns provided by the user
  for (size_t i = 0; i < fPhysicalVolumeNameRegexes.size(); ++i) {
    RMGLog::OutFormat(RMGLog::detail, "Physical volumes matching pattern '{}'['{}']",
        fPhysicalVolumeNameRegexes.at(i).c_str(), fPhysicalVolumeCopyNrRegexes.at(i).c_str());

    G4bool found = false;
    // scan the volume store for matches
    for (auto&& it = volume_store->begin(); it != volume_store->end(); it++) {
      if (std::regex_match((*it)->GetName(), std::regex(fPhysicalVolumeNameRegexes.at(i))) and
          std::regex_match(std::to_string((*it)->GetCopyNo()), std::regex(fPhysicalVolumeCopyNrRegexes.at(i)))) {

        fPhysicalVolumes.emplace_back(*it, G4RotationMatrix(), G4ThreeVector(), nullptr);

        RMGLog::OutFormat(RMGLog::detail, " · '{}[{}]', mass = {}", (*it)->GetName().c_str(),
            (*it)->GetCopyNo(), G4String(G4BestUnit(fPhysicalVolumes.data.back().volume, "Mass")));

        found = true;
      }
    }
    if (!found) {
      RMGLog::Out(RMGLog::warning, "No physical volumes names found matching pattern '",
          fPhysicalVolumeNameRegexes.at(i), "' and copy numbers matching pattern '",
          fPhysicalVolumeCopyNrRegexes.at(i), "'");
    }
  }

  if (fPhysicalVolumes.empty()) {
    RMGLog::Out(RMGLog::error, "No physical volumes names found matching any of the specified patterns");
    return;
  }

  auto world_volume = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();
  if (!world_volume) RMGLog::Out(RMGLog::fatal, "World volume not defined");

  // now inspect the solids the physical volumes refer to and configure the
  // appropriate sampling strategy
  for (auto&& el : fPhysicalVolumes.data) {

    // warning if world volume is selected
    if (el.physical_volume == world_volume) {
      RMGLog::Out(RMGLog::warning, "Do you really want to sample from the world volume?");
    }

    auto log_vol = el.physical_volume->GetLogicalVolume();
    auto solid = log_vol->GetSolid();
    auto solid_type = log_vol->GetSolid()->GetEntityType();

    // if the solid is simple one can avoid using bounding volumes for sampling
    // both volume and native surface sampling are available
    if (RMGGeneratorUtil::IsSampleable(solid_type)) {
      el.sampling_solid = solid;
      // if there are no daugthers one can avoid doing containment checks
      if (log_vol->GetNoDaughters()) el.containment_check = false;
      else el.containment_check = true;
    }
    // if it's not sampleable, cannot perform native surface sampling
    else if (fOnSurface) {
      RMGLog::Out(RMGLog::fatal, "Physical volume '", el.physical_volume->GetName(),
          "' does not satisfy requirements for native surface sampling");
    }
    // if we have a subtraction solid and the first one is supported for
    // sampling, use it but check for containment
    else if (solid_type == "G4SubtractionSolid") {
      if (RMGGeneratorUtil::IsSampleable(solid->GetConstituentSolid(0)->GetEntityType())) {
        el.sampling_solid = solid->GetConstituentSolid(0);
        el.containment_check = true;
      }
    }
    // use bounding solid for all other cases
    else {
      el.containment_check = true;
      auto solid_extent = solid->GetExtent(); // do not call multiple times, the function does not cache the result!
      if (fBoundingSolidType == "Sphere") {
        el.sampling_solid = new G4Orb("RMGGeneratorVolumeConfinement::fBoundingSphere", solid_extent.GetExtentRadius());
      }
      else if (fBoundingSolidType == "Box") {
        el.sampling_solid = new G4Box("RMGGeneratorVolumeConfinement::fBoundingBox",
            solid_extent.GetXmax() - solid_extent.GetXmin(),
            solid_extent.GetYmax() - solid_extent.GetYmin(),
            solid_extent.GetZmax() - solid_extent.GetZmin());
      }
      else {
        RMGLog::Out(RMGLog::fatal, "Bounding solid type '", fBoundingSolidType,
            "' not supported (implement me)");
      }
    }

    // determine solid transformation w.r.t. world volume reference

    G4ThreeVector vol_global_translation; // origin
    G4RotationMatrix vol_global_rotation; // identity
    std::vector<G4RotationMatrix> partial_rotations;
    std::vector<G4ThreeVector> partial_translations;

    for (auto v = el.physical_volume; v != world_volume; v = RMGNavigationTools::FindDirectMother(v)) {

      if (!v) RMGLog::Out(RMGLog::fatal, "nullptr detected in loop condition, this is unexpected. ",
          "Blame RMGPhysVolNavigator::FindDirectMother?");

      partial_rotations.emplace_back(*(v->GetObjectRotation()));
      partial_translations.push_back(v->GetObjectTranslation());

      vol_global_rotation = partial_rotations.back() * vol_global_rotation;
    }
    // world volume not included in loop
    partial_rotations.emplace_back();
    partial_translations.emplace_back(0, 0, 0);

    // partial_rotations[0] and partial_translations[0] refer to the target
    // volume partial_rotations[1] and partial_translations[1], to the direct
    // mother, etc. It is necessary to rotate with respect to the frame of the
    // mother. If there are no rotations (or only the target volume is
    // rotated): rotations are identity matrices and vol_global_translation =
    // sum(partial_translations)
    for (size_t i = 0; i < partial_translations.size()-1; i++) {
      G4ThreeVector tmp = partial_translations[i];
      for (size_t j = i+1; j < partial_rotations.size()-1; j++) {
        tmp *= partial_rotations[j];
      }
      vol_global_translation += tmp;
    }

    // assign transformation to sampling solid
    el.rotation = vol_global_rotation;
    el.translation = vol_global_translation;
  }
}

void RMGGeneratorVolumeConfinement::InitializeGeometricalVolumes() {

  if (!fGeomVolumeSolids.empty()) return;

  for (const auto& d : fGeomVolumeData) {
    if (d.g4_name == "Sphere") {
      fGeomVolumeSolids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Sphere("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
          d.sphere_inner_radius, d.sphere_outer_radius, 0, CLHEP::twopi, 0, CLHEP::pi));
    }
    else if (d.g4_name == "Cylinder") {
      fGeomVolumeSolids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Tubs("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
          d.cylinder_inner_radius, d.cylinder_outer_radius, 0.5*d.cylinder_height,
          d.cylinder_starting_angle, d.cylinder_spanning_angle));
    }
    else if (d.g4_name == "Box") {
      fGeomVolumeSolids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Box("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
          0.5*d.box_x_length, 0.5*d.box_y_length, 0.5*d.box_z_length));
    }
    // else if (...)
    else {
      RMGLog::Out(RMGLog::error, "Geometrical solid '", d.g4_name, "' not known! (Implement me?)");
    }

    RMGLog::OutFormat(RMGLog::detail, "Added geometrical solid of type '%s' with volume %g cm3 and surface %g cm2",
        fGeomVolumeSolids.back().sampling_solid->GetEntityType().c_str(),
        fGeomVolumeSolids.back().volume/CLHEP::cm3,
        fGeomVolumeSolids.back().surface/CLHEP::cm2);
  }
}

void RMGGeneratorVolumeConfinement::Reset() {
  fPhysicalVolumeNameRegexes.clear();
  fPhysicalVolumeCopyNrRegexes.clear();
  fPhysicalVolumes.clear();
  fGeomVolumeData.clear();
  fGeomVolumeSolids.clear();
  fSamplingMode = RMGGeneratorVolumeConfinement::kUnionAll;
  fOnSurface = false;
  fBoundingSolidType = "Sphere";
}

G4ThreeVector RMGGeneratorVolumeConfinement::ShootPrimaryPosition() {

  this->InitializePhysicalVolumes();
  this->InitializeGeometricalVolumes();

  switch (fSamplingMode) {
    case SamplingMode::kIntersectPhysicalWithGeometrical : {
      // strategy: sample a point in the geometrical user volume or the
      // physical user volume depending on the smallest surface/volume. Then
      // check if it is inside the other volume

      if (fGeomVolumeSolids.empty() or fPhysicalVolumes.empty()) {
        RMGLog::Out(RMGLog::fatal, "'IntersectPhysicalWithGeometrical' mode is set but ",
            "either no physical or no geometrical volumes have been added");
      }

      // choose a volume component randomly
      SampleableObject choice;
      G4bool physical_first;
      if (fOnSurface) {
        physical_first = fGeomVolumeSolids.total_surface > fPhysicalVolumes.total_surface;
        choice = physical_first ?
          fPhysicalVolumes.SurfaceWeightedRand() :
          fGeomVolumeSolids.SurfaceWeightedRand();
      }
      else {
        physical_first = fGeomVolumeSolids.total_volume > fPhysicalVolumes.total_volume;
        choice = physical_first ?
          fPhysicalVolumes.VolumeWeightedRand() :
          fGeomVolumeSolids.VolumeWeightedRand();
      }

      // shoot in the first region
      G4ThreeVector vertex;
      G4int calls = 0;
      while (calls++ < RMGVGeneratorPrimaryPosition::fMaxAttempts) {

        if (choice.containment_check) { // this can effectively happen only with physical volumes
          while (!fPhysicalVolumes.IsInside(vertex) and calls++ < RMGVGeneratorPrimaryPosition::fMaxAttempts) {
            vertex = choice.translation + choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
          }
          if (calls >= RMGVGeneratorPrimaryPosition::fMaxAttempts) {
            RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
                RMGVGeneratorPrimaryPosition::fMaxAttempts, "), check that your volumes are efficiently sampleable and ",
                "try, eventually, to increase the threshold through the dedicated macro command. Returning dummy vertex");
            return RMGVGeneratorPrimaryPosition::kDummyPrimaryPosition;
          }
        }
        else {
          vertex = choice.translation + choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
        }

        // is it also in the other volume class (geometrical/physical)?
        if (physical_first) { if (fGeomVolumeSolids.IsInside(vertex)) return vertex; }
        else { if (fPhysicalVolumes.IsInside(vertex)) return vertex; }
      }

      if (calls >= RMGVGeneratorPrimaryPosition::fMaxAttempts) {
        RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
            RMGVGeneratorPrimaryPosition::fMaxAttempts, "), check that your volumes are efficiently sampleable and ",
            "try, eventually, to increase the threshold through the dedicated macro command. Returning dummy vertex");
      }

      // everything has failed so return the dummy vertex
      return RMGVGeneratorPrimaryPosition::kDummyPrimaryPosition;
      break;
    }
    case SamplingMode::kUnionAll :

      if (fGeomVolumeSolids.empty() and fPhysicalVolumes.empty()) {
        RMGLog::Out(RMGLog::fatal, "'UnionAll' mode is set but ",
            "no physical or no geometrical volumes have been added");
      }

      auto choice = fOnSurface ?
        fPhysicalVolumes.SurfaceWeightedRand() :
        fPhysicalVolumes.VolumeWeightedRand();

      G4ThreeVector vertex;
      G4int calls = 0;
      while (calls++ < RMGVGeneratorPrimaryPosition::fMaxAttempts) {

        if (choice.containment_check) {
          while (!fPhysicalVolumes.IsInside(vertex) and calls++ < RMGVGeneratorPrimaryPosition::fMaxAttempts) {
            vertex = choice.translation + choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
          }
          if (calls >= RMGVGeneratorPrimaryPosition::fMaxAttempts) {
            RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
                RMGVGeneratorPrimaryPosition::fMaxAttempts, "), check that your volumes are efficiently sampleable and ",
                "try, eventually, to increase the threshold through the dedicated macro command. Returning dummy vertex");
            return RMGVGeneratorPrimaryPosition::kDummyPrimaryPosition;
          }
        }
        else {
          vertex = choice.translation + choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
        }

        return vertex;
      }

      if (calls >= RMGVGeneratorPrimaryPosition::fMaxAttempts) {
        RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
            RMGVGeneratorPrimaryPosition::fMaxAttempts, "), check that your volumes are efficiently sampleable and ",
            "try, eventually, to increase the threshold through the dedicated macro command. Returning dummy vertex");
      }

      // everything has failed so return the dummy vertex
      return RMGVGeneratorPrimaryPosition::kDummyPrimaryPosition;
      break;
  }

  return G4ThreeVector();
}

void RMGGeneratorVolumeConfinement::SetSamplingModeString(G4String mode) {
  try { this->SetSamplingMode(RMGTools::ToEnum<RMGGeneratorVolumeConfinement::SamplingMode>(mode, "sampling mode")); }
  catch (const std::bad_cast&) { return; }
}

void RMGGeneratorVolumeConfinement::AddPhysicalVolumeString(G4String expr) {
  if (expr.find(' ') == std::string::npos) this->AddPhysicalVolumeNameRegex(expr);
  else {
    auto name = expr.substr(0, expr.find_first_of(' '));
    auto copy_nr = expr.substr(expr.find_first_of(' ')+1, std::string::npos);
    this->AddPhysicalVolumeNameRegex(name, copy_nr);
  }
}

void RMGGeneratorVolumeConfinement::AddGeometricalVolumeString(G4String solid) {
  GenericGeometricalSolidData data;
  data.g4_name = solid;
  fGeomVolumeData.push_back(data);
}

RMGGeneratorVolumeConfinement::GenericGeometricalSolidData& RMGGeneratorVolumeConfinement::SafeBack() {
  if (fGeomVolumeData.empty()) {
    RMGLog::Out(RMGLog::fatal, "Must call /RMG/Generator/Confinement/Geometrical/AddSolid",
        "' before setting any geometrical parameter value");
  }
  return fGeomVolumeData.back();
};

void RMGGeneratorVolumeConfinement::DefineCommands() {

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/",
      "Commands for controlling primary confinement"));

  fMessengers.back()->DeclareMethod("SamplingMode", &RMGGeneratorVolumeConfinement::SetSamplingModeString)
    .SetGuidance("Select sampling mode for volume confinement")
    .SetParameterName("mode", false)
    .SetCandidates(RMGTools::GetCandidates<RMGGeneratorVolumeConfinement::SamplingMode>())
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethod("FallbackBoundingVolumeType", &RMGGeneratorVolumeConfinement::SetBoundingSolidType)
    .SetGuidance("Select fallback bounding volume type for complex solids")
    .SetParameterName("solid", false)
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareProperty("MaxSamplingTrials", fMaxAttempts)
    .SetGuidance("Set maximum number of attempts for sampling primary positions in a volume")
    .SetParameterName("N", false)
    .SetRange("N > 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Physical/",
      "Commands for setting physical volumes up for primary confinement"));

  fMessengers.back()->DeclareMethod("AddVolume", &RMGGeneratorVolumeConfinement::AddPhysicalVolumeString)
    .SetGuidance("Add physical volume to sample primaries from")
    .SetParameterName("regex", false)
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/",
      "Commands for setting geometrical volumes up for primary confinement"));

  fMessengers.back()->DeclareMethod("AddSolid", &RMGGeneratorVolumeConfinement::AddGeometricalVolumeString)
    .SetGuidance("Add geometrical solid to sample primaries from")
    .SetParameterName("solid", false)
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethodWithUnit("CenterPosition", "cm", &RMGGeneratorVolumeConfinement::SetGeomVolumeCenter)
    .SetGuidance("Set center position")
    .SetParameterName("point", false)
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Sphere/",
      "Commands for setting geometrical dimensions of a sampling sphere"));

  fMessengers.back()->DeclareMethodWithUnit("InnerRadius", "cm", &RMGGeneratorVolumeConfinement::SetGeomSphereInnerRadius)
    .SetGuidance("Set inner radius")
    .SetParameterName("L", false)
    .SetRange("L >= 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethodWithUnit("OuterRadius", "cm", &RMGGeneratorVolumeConfinement::SetGeomSphereInnerRadius)
    .SetGuidance("Set outer radius")
    .SetParameterName("L", false)
    .SetRange("L > 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Cylinder/",
      "Commands for setting geometrical dimensions of a sampling cylinder"));

  fMessengers.back()->DeclareMethodWithUnit("InnerRadius", "cm", &RMGGeneratorVolumeConfinement::SetGeomCylinderInnerRadius)
    .SetGuidance("Set inner radius")
    .SetParameterName("L", false)
    .SetRange("L >= 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethodWithUnit("OuterRadius", "cm", &RMGGeneratorVolumeConfinement::SetGeomCylinderOuterRadius)
    .SetGuidance("Set outer radius")
    .SetParameterName("L", false)
    .SetRange("L > 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethodWithUnit("Height", "cm", &RMGGeneratorVolumeConfinement::SetGeomCylinderHeight)
    .SetGuidance("Set height")
    .SetParameterName("L", false)
    .SetRange("L > 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethodWithUnit("StartingAngle", "cm", &RMGGeneratorVolumeConfinement::SetGeomCylinderStartingAngle)
    .SetGuidance("Set starting angle")
    .SetParameterName("A", false)
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethodWithUnit("SpanningAngle", "cm", &RMGGeneratorVolumeConfinement::SetGeomCylinderSpanningAngle)
    .SetGuidance("Set spanning angle")
    .SetParameterName("A", false)
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Box/",
      "Commands for setting geometrical dimensions of a sampling box"));

  fMessengers.back()->DeclareMethodWithUnit("XLength", "cm", &RMGGeneratorVolumeConfinement::SetGeomBoxXLength)
    .SetGuidance("Set X length")
    .SetParameterName("L", false)
    .SetRange("L > 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethodWithUnit("YLength", "cm", &RMGGeneratorVolumeConfinement::SetGeomBoxYLength)
    .SetGuidance("Set Y length")
    .SetParameterName("L", false)
    .SetRange("L > 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);

  fMessengers.back()->DeclareMethodWithUnit("ZLength", "cm", &RMGGeneratorVolumeConfinement::SetGeomBoxZLength)
    .SetGuidance("Set Z length")
    .SetParameterName("L", false)
    .SetRange("L > 0")
    .SetStates(G4State_Idle)
    .SetToBeBroadcasted(true);
}

// vim: tabstop=2 shiftwidth=2 expandtab
