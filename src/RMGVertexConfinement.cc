#include "RMGVertexConfinement.hh"

#include "G4Box.hh"
#include "G4GenericMessenger.hh"
#include "G4Orb.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4TransportationManager.hh"
#include "G4Tubs.hh"
#include "G4UnitsTable.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VisExtent.hh"
#include "Randomize.hh"

#include "RMGGeneratorUtil.hh"
#include "RMGLog.hh"
#include "RMGManager.hh"
#include "RMGNavigationTools.hh"

#include "RMGTools.hh"

// This structure must contain at least a non-null pointer, between the first
// and the last argument. The idea is that
//  - physical volumes get always a bounding box assigned, but later
//  - purely geometrical volumes only have the G4VSolid member defined
RMGVertexConfinement::SampleableObject::SampleableObject(G4VPhysicalVolume* v, G4RotationMatrix r,
    G4ThreeVector t, G4VSolid* s)
    :

      rotation(r), translation(t) {

  // at least one volume must be specified
  if (!v and !s) RMGLog::Out(RMGLog::error, "Invalid pointers given to constructor");

  this->physical_volume = v;
  this->sampling_solid = s;

  // should use the physical volume properties, if available
  const auto& solid =
      physical_volume ? physical_volume->GetLogicalVolume()->GetSolid() : sampling_solid;

  this->volume = solid->GetCubicVolume();
  this->surface = solid->GetSurfaceArea();
}

const RMGVertexConfinement::SampleableObject& RMGVertexConfinement::SampleableObjectCollection::
    SurfaceWeightedRand() const {

  if (data.empty()) RMGLog::OutDev(RMGLog::fatal, "Cannot sample from an empty collection");

  auto choice = this->total_surface * G4UniformRand();
  double w = 0;
  for (const auto& o : this->data) {
    if (choice > w and choice <= w + o.surface) return o;
    w += o.surface;
    if (w >= this->total_surface) {
      RMGLog::Out(RMGLog::error, "Sampling from collection of sampleables failed unexpectedly ",
          "(out-of-range error). Returning last object");
      return this->data.back();
    }
  }
  return this->data.back();
}

const RMGVertexConfinement::SampleableObject& RMGVertexConfinement::SampleableObjectCollection::
    VolumeWeightedRand() const {

  if (data.empty()) RMGLog::OutDev(RMGLog::fatal, "Cannot sample from an empty collection");

  auto choice = this->total_volume * G4UniformRand();
  double w = 0;
  for (const auto& o : this->data) {
    if (choice > w and choice <= w + o.volume) return o;
    w += o.volume;
    if (w >= this->total_volume) {
      RMGLog::Out(RMGLog::error, "Sampling from collection of sampleables failed unexpectedly ",
          "(out-of-range error). Returning last object");
      return this->data.back();
    }
  }
  return this->data.back();
}

bool RMGVertexConfinement::SampleableObjectCollection::IsInside(const G4ThreeVector& vertex) const {
  auto navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  for (const auto& o : data) {
    if (o.physical_volume) {
      if (navigator->LocateGlobalPointAndSetup(vertex) == o.physical_volume) return true;
    } else {
      if (o.sampling_solid->Inside(o.rotation.inverse() * vertex - o.translation)) return true;
    }
  }
  return false;
}

void RMGVertexConfinement::SampleableObjectCollection::emplace_back(G4VPhysicalVolume* v,
    const G4RotationMatrix& r, const G4ThreeVector& t, G4VSolid* s) {

  this->data.emplace_back(v, r, t, s);

  const auto& _v = this->data.back().volume;
  const auto& _s = this->data.back().surface;

  if (_v > 0) this->total_volume += _v;
  else
    RMGLog::Out(RMGLog::error, "One of the sampled solids has no volume attribute, ",
        "will not add it to the total volume of the collection. ",
        "this will affect sampling from multiple solids.");

  if (_s > 0) this->total_surface += _s;
  else
    RMGLog::Out(RMGLog::error, "One of the sampled solids has no surface attribute, ",
        "will not add it to the total surface of the collection. ",
        "this will affect sampling from multiple solids.");
}

/* ========================================================================================== */

RMGVertexConfinement::RMGVertexConfinement() : RMGVVertexGenerator("VolumeConfinement") {

  this->DefineCommands();
}

void RMGVertexConfinement::InitializePhysicalVolumes() {

  if (!fPhysicalVolumes.empty() or fPhysicalVolumeNameRegexes.empty()) return;

  auto volume_store = G4PhysicalVolumeStore::GetInstance();

  // scan all search patterns provided by the user
  for (size_t i = 0; i < fPhysicalVolumeNameRegexes.size(); ++i) {
    RMGLog::OutFormat(RMGLog::detail, "Physical volumes matching pattern '{}'['{}']",
        fPhysicalVolumeNameRegexes.at(i).c_str(), fPhysicalVolumeCopyNrRegexes.at(i).c_str());

    bool found = false;
    // scan the volume store for matches
    for (auto&& it = volume_store->begin(); it != volume_store->end(); it++) {
      if (std::regex_match((*it)->GetName(), std::regex(fPhysicalVolumeNameRegexes.at(i))) and
          std::regex_match(std::to_string((*it)->GetCopyNo()),
              std::regex(fPhysicalVolumeCopyNrRegexes.at(i)))) {

        // insert it in our collection
        // do not specify a bounding solid at this stage
        fPhysicalVolumes.emplace_back(*it, G4RotationMatrix(), G4ThreeVector(), nullptr);

        RMGLog::OutFormat(RMGLog::detail, " · '{}[{}]', mass = {}", (*it)->GetName().c_str(),
            (*it)->GetCopyNo(), std::string(G4BestUnit(fPhysicalVolumes.data.back().volume, "Mass")));

        found = true;
      }
    }
    if (!found) {
      RMGLog::Out(RMGLog::warning, "No physical volumes names found matching pattern '",
          fPhysicalVolumeNameRegexes.at(i), "' and copy numbers matching pattern '",
          fPhysicalVolumeCopyNrRegexes.at(i), "'");
    }
  }

  if (!fPhysicalVolumeNameRegexes.empty() and fPhysicalVolumes.empty()) {
    RMGLog::Out(RMGLog::error,
        "No physical volumes names found matching any of the specified patterns");
    return;
  }

  RMGLog::Out(RMGLog::detail, "Will sample points in the ", fOnSurface ? "surface" : "bulk",
      " of physical volumes");

  auto world_volume =
      G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume();
  if (!world_volume) RMGLog::Out(RMGLog::fatal, "World volume not defined");

  // now inspect the solids the physical volumes refer to and configure the
  // appropriate sampling strategy, i.e. setting the sampling solid and
  // containment check flag
  for (auto&& el : fPhysicalVolumes.data) {

    RMGLog::OutFormatDev(RMGLog::debug, "Inspecting volume '{}'", el.physical_volume->GetName());

    // warning if world volume is selected
    if (el.physical_volume == world_volume) {
      RMGLog::Out(RMGLog::warning, "Do you really want to sample from the world volume?");
    }

    auto log_vol = el.physical_volume->GetLogicalVolume();
    auto solid = log_vol->GetSolid();
    auto solid_type = log_vol->GetSolid()->GetEntityType();

    // if the solid is simple one can avoid using bounding volumes for
    // sampling.  Both volume and native surface sampling are available
    if (RMGGeneratorUtil::IsSampleable(solid_type)) {
      RMGLog::OutDev(RMGLog::debug, "Is sampleable natively (no bounding boxes)");
      el.sampling_solid = solid;
      // if there are no daugthers one can avoid doing containment checks
      if (log_vol->GetNoDaughters() > 0) {
        RMGLog::OutDev(RMGLog::debug, "Has daughters, containment check needed");
        el.containment_check = true;
      } else {
        RMGLog::OutDev(RMGLog::debug, "Has no daughters, no containment check needed");
        el.containment_check = false;
      }
    }
    // if it's not sampleable, cannot perform native surface sampling
    else if (fOnSurface) {
      RMGLog::Out(RMGLog::fatal, "Physical volume '", el.physical_volume->GetName(),
          "' does not satisfy requirements for native surface sampling");
    }
    // if we have a subtraction solid and the first one is supported for
    // sampling, use it but check for containment
    else if (solid_type == "G4SubtractionSolid" and
             RMGGeneratorUtil::IsSampleable(solid->GetConstituentSolid(0)->GetEntityType())) {
      RMGLog::OutDev(RMGLog::debug,
          "Is a subtraction solid, sampling from constituent solid with containment check");
      el.sampling_solid = solid->GetConstituentSolid(0);
      el.containment_check = true;
    }
    // use bounding solid for all other cases
    else {
      RMGLog::OutDev(RMGLog::debug, "Is not sampleable natively, need a bounding solid with ",
          "containment check (currently a ", fBoundingSolidType, ")");
      el.containment_check = true;
      auto solid_extent =
          solid->GetExtent(); // do not call multiple times, the function does not cache the result!
      if (fBoundingSolidType == "Sphere") {
        el.sampling_solid =
            new G4Orb(el.physical_volume->GetName() + "/RMGVertexConfinement::fBoundingSphere",
                solid_extent.GetExtentRadius());
      } else if (fBoundingSolidType == "Box") {
        el.sampling_solid =
            new G4Box(el.physical_volume->GetName() + "/RMGVertexConfinement::fBoundingBox",
                solid_extent.GetXmax() - solid_extent.GetXmin(),
                solid_extent.GetYmax() - solid_extent.GetYmin(),
                solid_extent.GetZmax() - solid_extent.GetZmin());
      } else {
        RMGLog::Out(RMGLog::fatal, "Bounding solid type '", fBoundingSolidType,
            "' not supported (implement me)");
      }
    } // sampling_solid and containment_check must hold a valid value at this point

    // determine solid transformation w.r.t. world volume reference

    G4ThreeVector vol_global_translation; // origin
    G4RotationMatrix vol_global_rotation; // identity
    std::vector<G4RotationMatrix> partial_rotations;
    std::vector<G4ThreeVector> partial_translations;

    for (auto v = el.physical_volume; v != world_volume; v = RMGNavigationTools::FindDirectMother(v)) {

      if (!v)
        RMGLog::Out(RMGLog::fatal, "nullptr detected in loop condition, this is unexpected. ",
            "Blame RMGPhysVolNavigator::FindDirectMother?");

      partial_rotations.push_back(v->GetObjectRotationValue());
      partial_translations.push_back(v->GetObjectTranslation());

      vol_global_rotation = partial_rotations.back() * vol_global_rotation;
    }
    // world volume not included in loop
    partial_translations.emplace_back(); // origin
    partial_rotations.emplace_back();    // identity

    // partial_rotations[0] and partial_translations[0] refer to the target
    // volume partial_rotations[1] and partial_translations[1], to the direct
    // mother, etc. It is necessary to rotate with respect to the frame of the
    // mother. If there are no rotations (or only the target volume is
    // rotated): rotations are identity matrices and vol_global_translation =
    // sum(partial_translations)
    for (size_t i = 0; i < partial_translations.size() - 1; i++) {
      G4ThreeVector tmp = partial_translations[i];
      for (size_t j = i + 1; j < partial_rotations.size() - 1; j++) { tmp *= partial_rotations[j]; }
      vol_global_translation += tmp;
    }

    // assign transformation to sampling solid
    el.rotation = vol_global_rotation;
    el.translation = vol_global_translation;
  }
}

void RMGVertexConfinement::InitializeGeometricalVolumes() {

  // if collections are not empty, assume initialization to be already done and skip
  if (!fGeomVolumeSolids.empty() or fGeomVolumeData.empty()) return;

  // no physical volume is specified nor at initialization or later
  for (const auto& d : fGeomVolumeData) {
    if (d.g4_name == "Sphere") {
      fGeomVolumeSolids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Sphere("RMGVertexConfinement::fGeomSamplingShape::Sphere/" +
                           std::to_string(fGeomVolumeSolids.size() + 1),
              d.sphere_inner_radius, d.sphere_outer_radius, 0, CLHEP::twopi, 0, CLHEP::pi));
    } else if (d.g4_name == "Cylinder") {
      fGeomVolumeSolids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Tubs("RMGVertexConfinement::fGeomSamplingShape::Cylinder/" +
                         std::to_string(fGeomVolumeSolids.size() + 1),
              d.cylinder_inner_radius, d.cylinder_outer_radius, 0.5 * d.cylinder_height,
              d.cylinder_starting_angle, d.cylinder_spanning_angle));
    } else if (d.g4_name == "Box") {
      fGeomVolumeSolids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Box("RMGVertexConfinement::fGeomSamplingShape::Box/" +
                        std::to_string(fGeomVolumeSolids.size() + 1),
              0.5 * d.box_x_length, 0.5 * d.box_y_length, 0.5 * d.box_z_length));
    }
    // else if (...)
    else {
      RMGLog::OutFormat(RMGLog::error, "Geometrical solid '{}' not known! (Implement me?)",
          d.g4_name);
    }

    fGeomVolumeSolids.back().containment_check = false;

    RMGLog::Out(RMGLog::detail, "Added geometrical solid of type '",
        fGeomVolumeSolids.back().sampling_solid->GetEntityType(), "' with volume ",
        G4BestUnit(fGeomVolumeSolids.back().volume, "Volume"), "and surface ",
        G4BestUnit(fGeomVolumeSolids.back().surface, "Surface"));
  }

  RMGLog::Out(RMGLog::detail, "Will sample points in the ", fOnSurface ? "surface" : "bulk",
      " of geometrical volumes");
}

void RMGVertexConfinement::Reset() {
  fPhysicalVolumeNameRegexes.clear();
  fPhysicalVolumeCopyNrRegexes.clear();
  fPhysicalVolumes.clear();
  fGeomVolumeData.clear();
  fGeomVolumeSolids.clear();
  fSamplingMode = RMGVertexConfinement::kUnionAll;
  fOnSurface = false;
  fBoundingSolidType = "Sphere";
}

void RMGVertexConfinement::GeneratePrimariesVertex(G4ThreeVector& vertex) {

  // configure sampling volumes (does not do anything if this is not the first
  // call)
  this->InitializePhysicalVolumes();
  this->InitializeGeometricalVolumes();

  RMGLog::OutDev(RMGLog::debug,
      "Sampling mode: ", magic_enum::enum_name<RMGVertexConfinement::SamplingMode>(fSamplingMode));

  switch (fSamplingMode) {
    case SamplingMode::kIntersectPhysicalWithGeometrical: {
      // strategy: sample a point in the geometrical user volume or the
      // physical user volume depending on the smallest surface/volume. Then
      // check if it is inside the other volume

      if (fGeomVolumeSolids.empty() or fPhysicalVolumes.empty()) {
        RMGLog::Out(RMGLog::fatal, "'IntersectPhysicalWithGeometrical' mode is set but ",
            "either no physical or no geometrical volumes have been added");
      }

      // choose a volume component randomly
      SampleableObject choice_nonconst;
      bool physical_first;
      if (fOnSurface) {
        physical_first = fGeomVolumeSolids.total_surface > fPhysicalVolumes.total_surface;
        choice_nonconst = physical_first ? fPhysicalVolumes.SurfaceWeightedRand()
                                         : fGeomVolumeSolids.SurfaceWeightedRand();
      } else {
        physical_first = fGeomVolumeSolids.total_volume > fPhysicalVolumes.total_volume;
        choice_nonconst = physical_first ? fPhysicalVolumes.VolumeWeightedRand()
                                         : fGeomVolumeSolids.VolumeWeightedRand();
      }
      const auto& choice = choice_nonconst;

      // shoot in the first region
      int calls = 0;
      while (calls++ < RMGVVertexGenerator::fMaxAttempts) {

        if (choice.containment_check) { // this can effectively happen only with physical volumes, at the moment
          while (!fPhysicalVolumes.IsInside(vertex) and calls++ < RMGVVertexGenerator::fMaxAttempts) {
            vertex = choice.translation +
                     choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
          }
          if (calls >= RMGVVertexGenerator::fMaxAttempts) {
            RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
                RMGVVertexGenerator::fMaxAttempts, "), check that your volumes are efficiently ",
                "sampleable and try, eventually, to increase the threshold through the dedicated ",
                "macro command. Returning dummy vertex");
            vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
            return;
          }
        } else {
          vertex = choice.translation +
                   choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
        }

        // is it also in the other volume class (geometrical/physical)?
        if (physical_first) {
          if (fGeomVolumeSolids.IsInside(vertex)) return;
        } else {
          if (fPhysicalVolumes.IsInside(vertex)) return;
        }
      }

      if (calls >= RMGVVertexGenerator::fMaxAttempts) {
        RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
            RMGVVertexGenerator::fMaxAttempts, "), check that your volumes are efficiently ",
            "sampleable and try, eventually, to increase the threshold through the dedicated ",
            "macro command. Returning dummy vertex");
      }

      // everything has failed so return the dummy vertex
      vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
      return;
    }

    case SamplingMode::kUnionAll: {
      // strategy: just sample uniformly in/on all geometrical and physical volumes

      // merge everything in a single container and use that
      auto all_volumes = fGeomVolumeSolids;
      all_volumes.insert(fPhysicalVolumes);

      if (all_volumes.empty()) {
        RMGLog::Out(RMGLog::fatal, "'UnionAll' mode is set but ",
            "no physical or no geometrical volumes have been added");
      }

      const auto choice =
          fOnSurface ? all_volumes.SurfaceWeightedRand() : all_volumes.VolumeWeightedRand();

      if (choice.physical_volume) {
        RMGLog::OutFormatDev(RMGLog::debug, "Chosen random volume: '{}[{}]'",
            choice.physical_volume->GetName(), choice.physical_volume->GetCopyNo());
      } else {
        RMGLog::OutFormatDev(RMGLog::debug, "Chosen random volume: '{}'",
            choice.sampling_solid->GetName());
      }
      RMGLog::OutDev(RMGLog::debug,
          "Maximum attempts to find a good vertex: ", RMGVVertexGenerator::fMaxAttempts);

      int calls = 0;
      while (calls++ < RMGVVertexGenerator::fMaxAttempts) {

        if (choice.containment_check) {
          vertex = choice.translation +
                   choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
          while (!fPhysicalVolumes.IsInside(vertex) and calls++ < RMGVVertexGenerator::fMaxAttempts) {
            vertex = choice.translation +
                     choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
            RMGLog::OutDev(RMGLog::debug, "Vertex was not inside, new vertex: ", vertex / CLHEP::cm,
                " cm");
          }
          if (calls >= RMGVVertexGenerator::fMaxAttempts) {
            RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
                RMGVVertexGenerator::fMaxAttempts, "), check that your volumes are efficiently ",
                "sampleable and try, eventually, to increase the threshold through the dedicated ",
                "macro command. Returning dummy vertex");
            vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
            return;
          }
        } else {
          vertex = choice.translation +
                   choice.rotation * RMGGeneratorUtil::rand(choice.sampling_solid, fOnSurface);
          RMGLog::OutDev(RMGLog::debug, "Generated vertex: ", vertex / CLHEP::cm, " cm");
        }

        RMGLog::OutDev(RMGLog::debug, "Found good vertex ", vertex / CLHEP::cm, " cm", " after ",
            calls, " iterations, returning");
        return;
      }

      if (calls >= RMGVVertexGenerator::fMaxAttempts) {
        RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
            RMGVVertexGenerator::fMaxAttempts,
            "), check that your volumes are efficiently sampleable and ",
            "try, eventually, to increase the threshold through the dedicated macro command. "
            "Returning dummy vertex");
      }

      // everything has failed so return the dummy vertex
      vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
      return;
    }

    default: {
      RMGLog::Out(RMGLog::error, "Sampling mode not implemented, returning dummy vertex");
      vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
      return;
    }
  }
}

void RMGVertexConfinement::SetSamplingModeString(std::string mode) {
  try {
    this->SetSamplingMode(
        RMGTools::ToEnum<RMGVertexConfinement::SamplingMode>(mode, "sampling mode"));
  } catch (const std::bad_cast&) { return; }
}

void RMGVertexConfinement::AddPhysicalVolumeString(std::string expr) {
  if (expr.find(' ') == std::string::npos) this->AddPhysicalVolumeNameRegex(expr);
  else {
    auto name = expr.substr(0, expr.find_first_of(' '));
    auto copy_nr = expr.substr(expr.find_first_of(' ') + 1, std::string::npos);
    this->AddPhysicalVolumeNameRegex(name, copy_nr);
  }
}

void RMGVertexConfinement::AddGeometricalVolumeString(std::string solid) {
  GenericGeometricalSolidData data;
  data.g4_name = solid;
  fGeomVolumeData.push_back(data);
}

RMGVertexConfinement::GenericGeometricalSolidData& RMGVertexConfinement::SafeBack() {
  if (fGeomVolumeData.empty()) {
    RMGLog::Out(RMGLog::fatal, "Must call /RMG/Generator/Confinement/Geometrical/AddSolid",
        "' before setting any geometrical parameter value");
  }
  return fGeomVolumeData.back();
}

void RMGVertexConfinement::DefineCommands() {

  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/",
      "Commands for controlling primary confinement"));

  fMessengers.back()
      ->DeclareProperty("SampleOnSurface", fOnSurface)
      .SetGuidance("If true (or omitted argument), sample on the surface of solids")
      .SetParameterName("flag", true)
      .SetStates(G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.back()
      ->DeclareMethod("SamplingMode", &RMGVertexConfinement::SetSamplingModeString)
      .SetGuidance("Select sampling mode for volume confinement")
      .SetParameterName("mode", false)
      .SetCandidates(RMGTools::GetCandidates<RMGVertexConfinement::SamplingMode>())
      .SetStates(G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.back()
      ->DeclareMethod("FallbackBoundingVolumeType", &RMGVertexConfinement::SetBoundingSolidType)
      .SetGuidance("Select fallback bounding volume type for complex solids")
      .SetParameterName("solid", false)
      .SetStates(G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.back()
      ->DeclareProperty("MaxSamplingTrials", fMaxAttempts)
      .SetGuidance("Set maximum number of attempts for sampling primary positions in a volume")
      .SetParameterName("N", false)
      .SetRange("N > 0")
      .SetStates(G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Physical/",
          "Commands for setting physical volumes up for primary confinement"));

  fMessengers.back()
      ->DeclareMethod("AddVolume", &RMGVertexConfinement::AddPhysicalVolumeString)
      .SetGuidance("Add physical volume to sample primaries from")
      .SetParameterName("regex", false)
      .SetStates(G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/",
          "Commands for setting geometrical volumes up for primary confinement"));

  fMessengers.back()
      ->DeclareMethod("AddSolid", &RMGVertexConfinement::AddGeometricalVolumeString)
      .SetGuidance("Add geometrical solid to sample primaries from")
      .SetParameterName("solid", false)
      .SetStates(G4State_Idle)
      .SetToBeBroadcasted(true);

  // FIXME: see comment in .hh
  fMessengers.back()
      ->DeclareMethodWithUnit("CenterPositionX", "cm", &RMGVertexConfinement::SetGeomVolumeCenterX)
      .SetGuidance("Set center position (X coordinate")
      .SetParameterName("value", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("CenterPositionY", "cm", &RMGVertexConfinement::SetGeomVolumeCenterY)
      .SetGuidance("Set center position (Y coordinate")
      .SetParameterName("value", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("CenterPositionZ", "cm", &RMGVertexConfinement::SetGeomVolumeCenterZ)
      .SetGuidance("Set center position (Z coordinate")
      .SetParameterName("value", false)
      .SetStates(G4State_Idle);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Sphere/",
          "Commands for setting geometrical dimensions of a sampling sphere"));

  fMessengers.back()
      ->DeclareMethodWithUnit("InnerRadius", "cm", &RMGVertexConfinement::SetGeomSphereInnerRadius)
      .SetGuidance("Set inner radius")
      .SetParameterName("L", false)
      .SetRange("L >= 0")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("OuterRadius", "cm", &RMGVertexConfinement::SetGeomSphereOuterRadius)
      .SetGuidance("Set outer radius")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_Idle);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Cylinder/",
          "Commands for setting geometrical dimensions of a sampling cylinder"));

  fMessengers.back()
      ->DeclareMethodWithUnit("InnerRadius", "cm", &RMGVertexConfinement::SetGeomCylinderInnerRadius)
      .SetGuidance("Set inner radius")
      .SetParameterName("L", false)
      .SetRange("L >= 0")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("OuterRadius", "cm", &RMGVertexConfinement::SetGeomCylinderOuterRadius)
      .SetGuidance("Set outer radius")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("Height", "cm", &RMGVertexConfinement::SetGeomCylinderHeight)
      .SetGuidance("Set height")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("StartingAngle", "deg",
          &RMGVertexConfinement::SetGeomCylinderStartingAngle)
      .SetGuidance("Set starting angle")
      .SetParameterName("A", false)
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("SpanningAngle", "deg",
          &RMGVertexConfinement::SetGeomCylinderSpanningAngle)
      .SetGuidance("Set spanning angle")
      .SetParameterName("A", false)
      .SetStates(G4State_Idle);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Box/",
          "Commands for setting geometrical dimensions of a sampling box"));

  fMessengers.back()
      ->DeclareMethodWithUnit("XLength", "cm", &RMGVertexConfinement::SetGeomBoxXLength)
      .SetGuidance("Set X length")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("YLength", "cm", &RMGVertexConfinement::SetGeomBoxYLength)
      .SetGuidance("Set Y length")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("ZLength", "cm", &RMGVertexConfinement::SetGeomBoxZLength)
      .SetGuidance("Set Z length")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab
