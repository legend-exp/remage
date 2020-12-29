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

#include "RMGGeneratorVolumeConfinementMessenger.hh"
#include "RMGGeneratorUtil.hh"
#include "RMGLog.hh"
#include "RMGNavigationTools.hh"
#include "RMGManager.hh"

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

  fG4Messenger = new RMGGeneratorVolumeConfinementMessenger(this) ;
}

void RMGGeneratorVolumeConfinement::InitializePhysicalVolumes() {

  if (!fPhysicalVolumes.empty()) return;

  auto volume_store = G4PhysicalVolumeStore::GetInstance();

  // scan all search patterns provided by the user
  for (size_t i = 0; i < fPhysicalVolumeNameRegexes.size(); ++i) {
    RMGLog::OutFormat(RMGLog::detail, "Physical volumes matching pattern '%s'['%s']",
        fPhysicalVolumeNameRegexes.at(i).c_str(), fPhysicalVolumeCopyNrRegexes.at(i).c_str());

    G4bool found = false;
    // scan the volume store for matches
    for (auto&& it = volume_store->begin(); it != volume_store->end(); it++) {
      if (std::regex_match((*it)->GetName(), std::regex(fPhysicalVolumeNameRegexes.at(i))) and
          std::regex_match(std::to_string((*it)->GetCopyNo()), std::regex(fPhysicalVolumeCopyNrRegexes.at(i)))) {

        fPhysicalVolumes.emplace_back(*it, G4RotationMatrix(), G4ThreeVector(), nullptr);

        RMGLog::OutFormat(RMGLog::detail, "Mass of '%s[%s]' = %g kg", (*it)->GetName().c_str(),
            (*it)->GetCopyNo(), fPhysicalVolumes.data.back().volume/CLHEP::kg);

        found = true;
      }
      if (!found) {
        RMGLog::Out(RMGLog::warning, "No physical volumes names found matching pattern '",
            fPhysicalVolumeNameRegexes.at(i), "' and copy numbers matching pattern '",
            fPhysicalVolumeCopyNrRegexes.at(i), "'");
      }
    }
  }

  if (fPhysicalVolumes.empty()) {
    RMGLog::Out(RMGLog::fatal, "No physical volumes names found matching any of the specified patterns");
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
          while (fPhysicalVolumes.IsInside(vertex) and calls++ < RMGVGeneratorPrimaryPosition::fMaxAttempts) {
            vertex = choice.translation + choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
          }
          if (calls >= RMGVGeneratorPrimaryPosition::fMaxAttempts) {
            RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
                RMGVGeneratorPrimaryPosition::fMaxAttempts, "), check that your volumes are efficiently sampleable and ",
                "try, in case, to increase the threshold through the dedicated macro command. Returning dummy vertex");
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
            "try, in case, to increase the threshold through the dedicated macro command. Returning dummy vertex");
      }

      return RMGVGeneratorPrimaryPosition::kDummyPrimaryPosition;
      break;
    }
    case SamplingMode::kUnionAll :


      break;
  }

  return G4ThreeVector();
}

// vim: tabstop=2 shiftwidth=2 expandtab
