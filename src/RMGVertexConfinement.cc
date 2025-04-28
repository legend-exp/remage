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

#include "RMGVertexConfinement.hh"

#include <chrono>
#include <optional>

#include "G4AutoLock.hh"
#include "G4Box.hh"
#include "G4GenericMessenger.hh"
#include "G4Orb.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Run.hh"
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

G4Mutex RMGVertexConfinement::fGeometryMutex = G4MUTEX_INITIALIZER;
// state shared between threads and protected by fGeometryMutex. See .hh file for details!
RMGVertexConfinement::SampleableObjectCollection RMGVertexConfinement::fGeomVolumeSolids = {};
RMGVertexConfinement::SampleableObjectCollection RMGVertexConfinement::fExcludedGeomVolumeSolids = {};
RMGVertexConfinement::SampleableObjectCollection RMGVertexConfinement::fPhysicalVolumes = {};

bool RMGVertexConfinement::fVolumesInitialized = false;

RMGVertexConfinement::SampleableObject::SampleableObject(G4VPhysicalVolume* physvol,
    G4RotationMatrix rot, G4ThreeVector trans, G4VSolid* sample_solid, bool is_native_sampleable,
    bool on_surface)
    : rotation(rot), translation(trans), surface_sample(on_surface),
      native_sample(is_native_sampleable) {

  // at least one volume must be specified
  if (!physvol and !sample_solid)
    RMGLog::Out(RMGLog::error, "Invalid pointers given to constructor");

  this->physical_volume = physvol;
  this->sampling_solid = sample_solid;

  // should use the physical volume properties, if available
  const auto& solid = physvol ? physvol->GetLogicalVolume()->GetSolid() : sample_solid;

  // NOTE: these functions use Monte Carlo methods when the solid is complex. Also note, that
  // they are not thread-safe in all cases!
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

bool RMGVertexConfinement::SampleableObject::IsInside(const G4ThreeVector& vertex) const {
  auto navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();

  if (this->physical_volume) {
    if (navigator->LocateGlobalPointAndSetup(vertex) == this->physical_volume) return true;
  } else {
    if (this->sampling_solid->Inside(this->rotation.inverse() * vertex - this->translation))
      return true;
  }

  return false;
}

// get the intersections between the solid and an arbitrary line, defined by start and dir
std::vector<G4ThreeVector> RMGVertexConfinement::SampleableObject::GetIntersections(G4ThreeVector start,
    G4ThreeVector dir) const {

  // check if the physical volume exists
  if ((not this->physical_volume) or (not this->physical_volume->GetLogicalVolume()->GetSolid()))
    RMGLog::OutDev(RMGLog::fatal, "Cannot find number of intersections for a SampleableObject ",
        "where the physical volume is not set. This probably means you are trying to "
        "generically sample a geometrical volume which instead should be natively sampled");

  auto solid = this->physical_volume->GetLogicalVolume()->GetSolid();

  G4double dist = 0;
  std::vector<G4ThreeVector> intersections = {};
  G4ThreeVector int_point;

  int_point = start;
  int counter = 0;
  dist = 0;

  // Find the number of intersections between the shape and a line.
  // Uses the DistanceToIn and DistanceToOut methods to find the next intersection,
  // continually call these (depending on if we are inside or outside) until the distance
  // becomes kInfinity.

  while (dist < kInfinity) {

    dist = (counter % 2) == 0 ? solid->DistanceToIn(int_point, dir)
                              : solid->DistanceToOut(int_point, dir);

    int_point = int_point + dist * dir;

    if (dist < kInfinity) intersections.push_back(int_point);
    counter++;
  }

  if (intersections.size() % 2)
    RMGLog::OutDev(RMGLog::fatal, "Found ", intersections.size(), " intersections. However,",
        "the number should always be divisible by two.");

  return intersections;
}


// generate a random position and direction for the surface sampling
void RMGVertexConfinement::SampleableObject::GetDirection(G4ThreeVector& dir,
    G4ThreeVector& pos) const {

  if ((not this->physical_volume) or (not this->physical_volume->GetLogicalVolume()->GetSolid()))
    RMGLog::OutDev(RMGLog::fatal, "Cannot generate directions for a SampleableObject ",
        "where the physical volume is not set. This probably means you are trying to "
        "generically sample a geometrical volume which instead should be natively sampled");

  // Get the center and radius of a bounding sphere around the shape
  G4double bounding_radius =
      physical_volume->GetLogicalVolume()->GetSolid()->GetExtent().GetExtentRadius();

  G4ThreeVector barycenter =
      physical_volume->GetLogicalVolume()->GetSolid()->GetExtent().GetExtentCenter();

  // start on z-axis, pointing down.
  pos = G4ThreeVector(0.0, 0.0, bounding_radius);
  dir = G4ThreeVector(0.0, 0.0, -1.0);

  // push in rho direction by some impact parameter
  G4double disk_phi = 2.0 * CLHEP::pi * G4UniformRand();
  G4double disk_r = sqrt(G4UniformRand()) * bounding_radius;
  pos += G4ThreeVector(cos(disk_phi) * disk_r, sin(disk_phi) * disk_r, 0);

  // now rotate pos and dir by some random direction
  G4double theta = acos(2.0 * G4UniformRand() - 1.0);
  G4double phi = 2.0 * CLHEP::pi * G4UniformRand();

  pos.rotateY(theta);
  pos.rotateZ(phi);
  dir.rotateY(theta);
  dir.rotateZ(phi);

  // shift by the barycenter of the bounding sphere.
  pos += barycenter;
}


bool RMGVertexConfinement::SampleableObject::GenerateSurfacePoint(G4ThreeVector& vertex,
    size_t max_attempts, size_t n_max) const {

  size_t calls = 0;
  G4ThreeVector dir;
  G4ThreeVector pos;

  while (calls < max_attempts) {

    // chose a random line
    this->GetDirection(dir, pos);

    // find the intersections
    std::vector<G4ThreeVector> intersections = this->GetIntersections(pos, dir);

    if (intersections.empty()) continue;

    // The surface sampling algorithm returns N intersections.
    // We have to select one, to keep independence of the sampled points
    // and weight by the number of intersections.

    auto random_int = static_cast<size_t>(n_max * G4UniformRand());

    if (random_int <= intersections.size() - 1) {
      vertex = intersections[random_int];
      return true;
    }
    calls++;
  }

  RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (", max_attempts,
      "), check that your surfaces are efficiently ",
      "sampleable and try, eventually, to increase the threshold through the dedicated ",
      "macro command /RMG/Generator/Confinement/MaxSamplingTrials. Returning dummy vertex");

  return false;
}


bool RMGVertexConfinement::SampleableObject::Sample(G4ThreeVector& vertex, size_t max_attempts,
    bool force_containment_check, size_t& n_trials) const {

  if (this->physical_volume) {
    RMGLog::OutFormatDev(RMGLog::debug, "Chosen random volume: '{}[{}]'",
        this->physical_volume->GetName(), this->physical_volume->GetCopyNo());
  } else {
    RMGLog::OutFormatDev(RMGLog::debug, "Chosen random volume: '{}'",
        this->sampling_solid->GetName());
  }
  RMGLog::OutDev(RMGLog::debug, "Maximum attempts to find a good vertex: ", max_attempts);

  size_t calls = 0;

  // possible sampling strategies
  // 1) native sampling
  // 2) volume sampling with containment check
  // 3) general surface sampling

  if (this->native_sample) {
    vertex = this->translation +
             this->rotation * RMGGeneratorUtil::rand(this->sampling_solid, this->surface_sample);
    RMGLog::OutDev(RMGLog::debug, "Generated vertex: ", vertex / CLHEP::cm, " cm");
    if (force_containment_check && !this->IsInside(vertex)) {

      RMGLog::OutDev(RMGLog::error,
          "Generated vertex not inside sampling volumes (forced containment check): ",
          vertex / CLHEP::cm, " cm");
    }
  } else if (this->surface_sample) {
    bool success = this->GenerateSurfacePoint(vertex, max_attempts, this->max_num_intersections);
    vertex = this->translation + this->rotation * vertex;
    if (not success) return false;

    if (force_containment_check && !this->IsInside(vertex)) {
      auto local_pos = this->rotation.inverse() * vertex - this->translation;
      auto vol_type = this->physical_volume->GetLogicalVolume()->GetSolid()->Inside(local_pos);
      RMGLog::OutDev(RMGLog::error,
          "Generated vertex not inside sampling volumes (forced containment check): ",
          vertex / CLHEP::cm, " cm (", magic_enum::enum_name<EInside>(vol_type), ")");
    }

  } else {
    vertex = this->translation + this->rotation * RMGGeneratorUtil::rand(this->sampling_solid, false);

    while (!this->IsInside(vertex) and calls++ < max_attempts) {
      n_trials++;
      vertex =
          this->translation + this->rotation * RMGGeneratorUtil::rand(this->sampling_solid, false);
      RMGLog::OutDev(RMGLog::debug, "Vertex was not inside, new vertex: ", vertex / CLHEP::cm, " cm");
    }
    if (calls >= max_attempts) {
      RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (", max_attempts,
          "), check that your volumes are efficiently ",
          "sampleable and try, eventually, to increase the threshold through the dedicated ",
          "macro command. Returning dummy vertex");
      return false;
    }
  }

  RMGLog::OutDev(RMGLog::debug, "Found good vertex ", vertex / CLHEP::cm, " cm", " after ", calls,
      " iterations, returning");
  return true;
}

bool RMGVertexConfinement::SampleableObjectCollection::IsInside(const G4ThreeVector& vertex) const {
  return std::any_of(data.begin(), data.end(),
      [&vertex](const auto& o) { return o.IsInside(vertex); });
}


template<typename... Args>
void RMGVertexConfinement::SampleableObjectCollection::emplace_back(Args&&... args) {

  this->data.emplace_back(std::forward<Args>(args)...);

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

  RMGLog::OutFormat(RMGLog::detail, "Create RMGVertexConfinment object");
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

        RMGLog::OutFormat(RMGLog::detail, " · '{}[{}]', volume = {}", (*it)->GetName().c_str(),
            (*it)->GetCopyNo(),
            std::string(G4BestUnit(fPhysicalVolumes.data.back().volume, "Volume")));

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
  std::vector<SampleableObject> new_obj_from_inspection;
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
      // if there are no daughters one can avoid doing containment checks
      if (log_vol->GetNoDaughters() > 0) {
        RMGLog::OutDev(RMGLog::debug, "Has daughters, containment check needed");
        el.native_sample = false;
        if (fOnSurface)
          RMGLog::OutDev(RMGLog::warning, "Surface sampling is on the outside of the mother volume "
                                          "for volumes with daughters.");

      } else {
        RMGLog::OutDev(RMGLog::debug, "Has no daughters, no containment check needed");
        el.native_sample = true;
      }
      el.surface_sample = fOnSurface;
    }
    // if it's not sampleable, cannot perform native surface sampling
    else if (fOnSurface) {

      if (log_vol->GetNoDaughters() > 0)
        RMGLog::OutDev(RMGLog::warning,
            "Surface sampling is on the outside of the mother volume for volumes with daughters.");

      RMGLog::Out(RMGLog::debug, "Physical volume '", el.physical_volume->GetName(),
          "' does not satisfy requirements for native surface sampling so resort to general "
          "surface sampler");

      el.native_sample = false;
      el.surface_sample = true;
      el.max_num_intersections = fSurfaceSampleMaxIntersections;

      if (fSurfaceSampleMaxIntersections < 2)
        RMGLog::Out(RMGLog::fatal, " for generic surface sampling SurfaceSampleMaxIntersections, the maximum number of lines a ",
            "line can intersect with the surface must be set with "
            "/RMG/Generator/Confinement/SurfaceSampleMaxIntersections",
            "Note: this can be an overestimate.");

    }
    // if we have a subtraction solid and the first one is supported for
    // sampling, use it but check for containment
    else if (solid_type == "G4SubtractionSolid" and
             RMGGeneratorUtil::IsSampleable(solid->GetConstituentSolid(0)->GetEntityType())) {
      RMGLog::OutDev(RMGLog::debug,
          "Is a subtraction solid, sampling from constituent solid with containment check");
      el.sampling_solid = solid->GetConstituentSolid(0);
      el.native_sample = false;
      el.surface_sample = false;
    }
    // use bounding solid for all other cases
    else {
      RMGLog::OutDev(RMGLog::debug, "Is not sampleable natively, need a bounding box with ",
          "containment check");
      el.native_sample = false;
      el.surface_sample = false;
      // to get a guaranteed bounding solid we rely on G4VSolid::BoundingLimits()
      // the function, implemented for each G4 solid, calculates the dimensions
      // of a bounding box. NOTE: it's possible to obtain a radius through
      // G4VSolid::GetExtent::GetExtentRadius(), but it's always computed as
      // sqrt(dX^2 + dY^2 + dZ^2) / 2, i.e. the smallest sphere containing the
      // bounding box. Such a sphere is always less efficient than the box!

      G4ThreeVector lim_min, lim_max;
      // NOTE: do not call G4VSolid::BoundingLimits() multiple times, the
      // function does not cache the result!
      solid->BoundingLimits(lim_min, lim_max);

      RMGLog::OutDev(RMGLog::debug, "Bounding box coordinates: min = ", lim_min, ", max = ", lim_max);

      // the origin of the local coordinates of the non-sampleable solid is not necessarily at
      // its barycenter. However, the coordinate origin of a G4Box is always its barycenter.
      double bb_x = std::max(std::abs(lim_max.getX()), std::abs(lim_min.getX()));
      double bb_y = std::max(std::abs(lim_max.getY()), std::abs(lim_min.getY()));
      double bb_z = std::max(std::abs(lim_max.getZ()), std::abs(lim_min.getZ()));

      el.sampling_solid =
          new G4Box(el.physical_volume->GetName() + "/RMGVertexConfinement::fBoundingBox", bb_x,
              bb_y, bb_z);
    } // sampling_solid and native_sample and surface_sample must hold a valid value at this point

    // determine solid transformation w.r.t. world volume reference

    // found paths to the mother volume.
    std::vector<VolumeTreeEntry> trees;

    // queue for paths to the mother volume that still have to be searched.
    std::queue<VolumeTreeEntry> q;
    q.emplace(el.physical_volume);

    for (; !q.empty(); q.pop()) {
      auto v = q.front();

      if (!v.physvol)
        RMGLog::OutDev(RMGLog::fatal, "nullptr detected in loop condition, this is unexpected. ",
            "Blame RMGNavigationTools::FindDirectMother?");

      v.partial_rotations.push_back(v.physvol->GetObjectRotationValue());
      v.partial_translations.push_back(v.physvol->GetObjectTranslation());

      v.vol_global_rotation = v.partial_rotations.back() * v.vol_global_rotation;

      for (auto m : RMGNavigationTools::FindDirectMothers(v.physvol)) {
        if (m != world_volume) {
          auto v_m = VolumeTreeEntry(v); // create a copy of the current helper object.
          v_m.physvol = m;
          q.push(v_m);
        } else { // we finished that branch!
          trees.push_back(v);
        }
      }
    }

    RMGLog::OutFormatDev(RMGLog::debug, "Found {} ways to reach world volume from {}", trees.size(),
        el.physical_volume->GetName());

    // finalize all found paths to the mother volume.
    for (auto&& v : trees) {
      // world volume not included in loop
      v.partial_translations.emplace_back(); // origin
      v.partial_rotations.emplace_back();    // identity

      // partial_rotations[0] and partial_translations[0] refer to the target
      // volume partial_rotations[1] and partial_translations[1], to the direct
      // mother, etc. It is necessary to rotate with respect to the frame of the
      // mother. If there are no rotations (or only the target volume is
      // rotated): rotations are identity matrices and vol_global_translation =
      // sum(partial_translations)
      for (size_t i = 0; i < v.partial_translations.size() - 1; i++) {
        G4ThreeVector tmp = v.partial_translations[i];
        for (size_t j = i + 1; j < v.partial_rotations.size() - 1; j++) {
          tmp *= v.partial_rotations[j];
        }
        v.vol_global_translation += tmp;
      }
    }

    if (trees.empty())
      RMGLog::OutDev(RMGLog::fatal, "No path to world volume found, that should not be!");
    // assign first found transformation to current sampling solid
    el.rotation = trees[0].vol_global_rotation;
    el.translation = trees[0].vol_global_translation;

    // we found more than one path to the mother volume. Append them to the list of sampling
    // solids separately.
    for (size_t i = 1; i < trees.size(); ++i) {
      const auto& v = trees.at(i);
      new_obj_from_inspection.emplace_back(el.physical_volume, v.vol_global_rotation,
          v.vol_global_translation, el.sampling_solid, el.native_sample, el.surface_sample);
    }
  }

  // finally add all newly found sampling solids (this os not done directly above as this
  // invalidates old iterators).
  for (const auto& s : new_obj_from_inspection) { fPhysicalVolumes.emplace_back(s); }

  RMGLog::OutFormat(RMGLog::detail, "Sampling from {} physical volumes, volume = {}",
      fPhysicalVolumes.size(), std::string(G4BestUnit(fPhysicalVolumes.total_volume, "Volume")));
}

void RMGVertexConfinement::InitializeGeometricalVolumes(bool use_excluded_volumes) {

  // Select the appropriate containers based on the option

  auto& volume_solids = use_excluded_volumes ? fExcludedGeomVolumeSolids : fGeomVolumeSolids;
  const auto& volume_data = use_excluded_volumes ? fExcludedGeomVolumeData : fGeomVolumeData;

  // If collections are not empty, assume initialization to be already done and skip
  if (!volume_solids.empty() || volume_data.empty()) return;

  // Initialize volumes based on the data
  for (const auto& d : volume_data) {
    if (d.solid_type == GeometricalSolidType::kSphere) {
      volume_solids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Sphere("RMGVertexConfinement::SamplingShape::Sphere/" +
                           std::to_string(volume_solids.size() + 1),
              d.sphere_inner_radius, d.sphere_outer_radius, 0, CLHEP::twopi, 0, CLHEP::pi));
    } else if (d.solid_type == GeometricalSolidType::kCylinder) {
      volume_solids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Tubs("RMGVertexConfinement::SamplingShape::Cylinder/" +
                         std::to_string(volume_solids.size() + 1),
              d.cylinder_inner_radius, d.cylinder_outer_radius, 0.5 * d.cylinder_height,
              d.cylinder_starting_angle, d.cylinder_spanning_angle));
    } else if (d.solid_type == GeometricalSolidType::kBox) {
      volume_solids.emplace_back(nullptr, G4RotationMatrix(), d.volume_center,
          new G4Box("RMGVertexConfinement::SamplingShape::Box/" +
                        std::to_string(volume_solids.size() + 1),
              0.5 * d.box_x_length, 0.5 * d.box_y_length, 0.5 * d.box_z_length));
    } else {
      RMGLog::OutFormat(RMGLog::error, "Geometrical solid '{}' not known! (Implement me?)",
          magic_enum::enum_name<GeometricalSolidType>(d.solid_type));
    }

    volume_solids.back().native_sample = true;
    volume_solids.back().surface_sample = fOnSurface;

    RMGLog::Out(RMGLog::detail, "Added geometrical solid ",
        use_excluded_volumes ? "(excluded) " : " ", "of type '",
        volume_solids.back().sampling_solid->GetEntityType(), "' with volume ",
        G4BestUnit(volume_solids.back().volume, "Volume"), "and surface ",
        G4BestUnit(volume_solids.back().surface, "Surface"));
  }

  RMGLog::Out(RMGLog::detail, "Will sample points in the ", fOnSurface ? "surface" : "bulk",
      " of geometrical volumes");
}

void RMGVertexConfinement::Reset() {
  // take lock, just not to race with the fVolumesInitialized flag elsewhere.

  G4AutoLock lock(&fGeometryMutex);
  fVolumesInitialized = true;
  fPhysicalVolumes.clear();
  fGeomVolumeSolids.clear();
  fExcludedGeomVolumeSolids.clear();

  fPhysicalVolumeNameRegexes.clear();
  fPhysicalVolumeCopyNrRegexes.clear();
  fGeomVolumeData.clear();
  fExcludedGeomVolumeData.clear();

  fSamplingMode = SamplingMode::kUnionAll;
  fFirstSamplingVolumeType = VolumeType::kUnset;

  fOnSurface = false;
  fForceContainmentCheck = false;
}

bool RMGVertexConfinement::GenerateVertex(G4ThreeVector& vertex) {
  auto time_sampling_start = std::chrono::high_resolution_clock::now();

  bool res = ActualGenerateVertex(vertex);

  auto time_sampling_end = std::chrono::high_resolution_clock::now();
  fVertexGenerationTime +=
      std::chrono::duration_cast<std::chrono::nanoseconds>(time_sampling_end - time_sampling_start);

  return res;
}

bool RMGVertexConfinement::ActualGenerateVertex(G4ThreeVector& vertex) {
  // configure sampling volumes (does not do anything if this is not the first
  // call)
  if (!fVolumesInitialized) {
    G4AutoLock lock(&fGeometryMutex);
    if (!fVolumesInitialized) {
      this->InitializePhysicalVolumes();
      this->InitializeGeometricalVolumes(true);
      this->InitializeGeometricalVolumes(false);

      fVolumesInitialized = true;
    }
  }

  RMGLog::OutDev(RMGLog::debug,
      "Sampling mode: ", magic_enum::enum_name<SamplingMode>(fSamplingMode));

  switch (fSamplingMode) {
    case SamplingMode::kIntersectPhysicalWithGeometrical: {
      if (fGeomVolumeSolids.empty() or fPhysicalVolumes.empty()) {
        RMGLog::Out(RMGLog::fatal, "'IntersectPhysicalWithGeometrical' mode is set but ",
            "either no physical or no geometrical volumes have been added");
      }

      int calls = 0;

      while (calls <= RMGVVertexGenerator::fMaxAttempts) {
        calls++;

        // choose a volume
        SampleableObject choice_nonconst;
        bool physical_first = true;

        // for surface events the user has to chose which volume type to sample first
        if (fOnSurface) {

          if (fFirstSamplingVolumeType == VolumeType::kUnset)
            RMGLog::Out(RMGLog::fatal, "For surface sampling vertex confinment mode it is required ",
                "to set the type of volume to sample first (geometrical or physical).");


          physical_first = fFirstSamplingVolumeType == VolumeType::kPhysical;
          choice_nonconst = physical_first ? fPhysicalVolumes.SurfaceWeightedRand()
                                           : fGeomVolumeSolids.SurfaceWeightedRand();

        } else {
          // for volume sampling the user can specify the volume to sample first else the set
          // with smaller total volume is used
          physical_first = fFirstSamplingVolumeType == VolumeType::kUnset
                               ? fGeomVolumeSolids.total_volume > fPhysicalVolumes.total_volume
                               : fFirstSamplingVolumeType == VolumeType::kPhysical;

          choice_nonconst = physical_first ? fPhysicalVolumes.VolumeWeightedRand()
                                           : fGeomVolumeSolids.VolumeWeightedRand();
        }

        const auto& choice = choice_nonconst;

        // generate a candidate vertex
        bool success = choice.Sample(vertex, fMaxAttempts, fForceContainmentCheck, fTrials);

        if (!success) {
          RMGLog::Out(RMGLog::error, "Sampling unsuccessful return dummy vertex");
          vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
          return false;
        }

        // is it also in the other volume class (geometrical/physical)?
        if (physical_first) {
          if (fGeomVolumeSolids.IsInside(vertex)) return true;
        } else {
          if (fPhysicalVolumes.IsInside(vertex)) return true;
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
      return false;
    }

    case SamplingMode::kSubtractGeometrical: {
      if (fGeomVolumeSolids.empty() and fPhysicalVolumes.empty()) {
        RMGLog::Out(RMGLog::fatal, "'SubtractGeometrical' mode is set but ",
            " no physical and no geometrical volumes have been added");
      }

      bool has_physical = not fPhysicalVolumes.empty();
      bool has_geometrical = not fGeomVolumeSolids.empty();

      if (fFirstSamplingVolumeType == VolumeType::kPhysical and not has_physical)
        RMGLog::Out(RMGLog::fatal, "Cannot sample first from physical volumes when none are set.");

      if (fFirstSamplingVolumeType == VolumeType::kGeometrical and not has_geometrical)
        RMGLog::Out(RMGLog::fatal,
            "Cannot sample first from geometrical volumes when none are set.");

      int calls = 0;

      while (calls <= RMGVVertexGenerator::fMaxAttempts) {
        calls++;

        // choose a volume
        SampleableObject choice_nonconst;
        bool physical_first = true;

        if (fOnSurface) {
          // check the user has set which volume to sample first from
          if (fFirstSamplingVolumeType == VolumeType::kUnset)
            RMGLog::Out(RMGLog::fatal, "For surface sampling vertex confinment mode it is required ",
                "to set the type of volume to sample first (geometrical or physical).");

          physical_first = fFirstSamplingVolumeType == VolumeType::kPhysical;


          choice_nonconst = physical_first ? fPhysicalVolumes.SurfaceWeightedRand()
                                           : fGeomVolumeSolids.SurfaceWeightedRand();
        } else {
          // if both physical and geometrical volumes are present and order is
          // not set choose based on the total volume
          if (fFirstSamplingVolumeType == VolumeType::kUnset && has_geometrical && has_physical)
            physical_first = fGeomVolumeSolids.total_volume > fPhysicalVolumes.total_volume;
          else if (has_physical && not has_geometrical) physical_first = true;
          else if (has_geometrical && not has_physical) physical_first = false;
          else physical_first = (fFirstSamplingVolumeType == VolumeType::kPhysical);

          choice_nonconst = physical_first ? fPhysicalVolumes.VolumeWeightedRand()
                                           : fGeomVolumeSolids.VolumeWeightedRand();
        }

        const auto& choice = choice_nonconst;

        // generate a candidate vertex
        bool success = choice.Sample(vertex, fMaxAttempts, fForceContainmentCheck, fTrials);

        if (!success) {
          RMGLog::Out(RMGLog::error, "Sampling unsuccessful, return dummy vertex");
          vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
          return false;
        }

        // check for intersections
        bool accept = false;

        if (physical_first && has_geometrical) {
          if (fGeomVolumeSolids.IsInside(vertex)) accept = true;
        } else if (not physical_first && has_physical) {
          if (fPhysicalVolumes.IsInside(vertex)) accept = true;
        } else {
          accept = true;
        }
        RMGLog::Out(RMGLog::debug, accept ? "Chosen vertex passes intersection criteria "
                                          : "Chosen vertex fails intersection criteria. ");

        // now check for subtractions
        if (accept && !fExcludedGeomVolumeSolids.IsInside(vertex)) return true;

        RMGLog::Out(RMGLog::debug, "Chosen vertex fails intersection criteria.");
      }

      if (calls >= RMGVVertexGenerator::fMaxAttempts) {
        RMGLog::Out(RMGLog::error, "Exceeded maximum number of allowed iterations (",
            RMGVVertexGenerator::fMaxAttempts, "), check that your volumes are efficiently ",
            "sampleable and try, eventually, to increase the threshold through the dedicated ",
            "macro command. Returning dummy vertex");
      }

      // everything has failed so return the dummy vertex
      vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
      return false;
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

      // chose a volume to sample from
      const auto choice =
          fOnSurface ? all_volumes.SurfaceWeightedRand() : all_volumes.VolumeWeightedRand();

      // do the sampling
      bool success = choice.Sample(vertex, fMaxAttempts, fForceContainmentCheck, fTrials);

      if (!success) {
        RMGLog::Out(RMGLog::error, "Sampling unsuccessful return dummy vertex");
        vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
        return false;
      }
      return true;
    }

    default: {
      RMGLog::Out(RMGLog::error, "Sampling mode not implemented, returning dummy vertex");
      vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
      return false;
    }
  }
}

void RMGVertexConfinement::SetSamplingModeString(std::string mode) {
  try {
    this->SetSamplingMode(RMGTools::ToEnum<SamplingMode>(mode, "sampling mode"));
  } catch (const std::bad_cast&) { return; }
}

void RMGVertexConfinement::SetFirstSamplingVolumeTypeString(std::string type) {
  try {
    this->SetFirstSamplingVolumeType(RMGTools::ToEnum<VolumeType>(type, "volume type"));
  } catch (const std::bad_cast&) { return; }
}

void RMGVertexConfinement::AddPhysicalVolumeNameRegex(std::string name, std::string copy_nr) {
  if (copy_nr.empty()) copy_nr = ".*"; // for default arg from messenger.
  if (!fPhysicalVolumes.empty()) {
    RMGLog::Out(RMGLog::fatal,
        "Volumes for vertex confinement have already been initialized, no change possible!");
  }
  fPhysicalVolumeNameRegexes.emplace_back(name);
  fPhysicalVolumeCopyNrRegexes.emplace_back(copy_nr);
}

void RMGVertexConfinement::AddGeometricalVolumeString(std::string solid) {
  GenericGeometricalSolidData data;
  data.solid_type = RMGTools::ToEnum<GeometricalSolidType>(solid, "solid type");
  fGeomVolumeData.push_back(data);
  fLastSolidExcluded = false;
}

void RMGVertexConfinement::AddExcludedGeometricalVolumeString(std::string solid) {
  GenericGeometricalSolidData data;
  data.solid_type = RMGTools::ToEnum<GeometricalSolidType>(solid, "solid type");
  fExcludedGeomVolumeData.push_back(data);
  fLastSolidExcluded = true;

  if (fSamplingMode != SamplingMode::kSubtractGeometrical) {
    RMGLog::Out(RMGLog::fatal, "Cannot set excluded geometrical regions in any other sampling mode",
        "' than kSubtractGeometrical");
  }
}

RMGVertexConfinement::GenericGeometricalSolidData& RMGVertexConfinement::SafeBack(
    std::optional<GeometricalSolidType> solid_type) {


  // checks - need to be performed both for GeomVolume and ExcludedGeomVolume
  auto& volume_solids = fLastSolidExcluded ? fExcludedGeomVolumeSolids : fGeomVolumeSolids;
  auto& volume_data = fLastSolidExcluded ? fExcludedGeomVolumeData : fGeomVolumeData;
  auto command_name = fLastSolidExcluded ? "AddExcludedSolid" : "AddSolid";

  if (volume_data.empty()) {
    RMGLog::Out(RMGLog::fatal, "Must call /RMG/Generator/Confinement/Geometrical/", command_name,
        "' before setting any geometrical parameter value");
  }

  if (!volume_solids.empty()) {
    RMGLog::Out(RMGLog::fatal,
        "Solids for vertex confinement have already been initialized, no change possible!");
  }


  if (solid_type.has_value() && volume_data.back().solid_type != solid_type) {
    RMGLog::OutFormat(RMGLog::fatal, "Trying to modify non-{0} as {0}",
        magic_enum::enum_name<GeometricalSolidType>(solid_type.value()));
  }

  return volume_data.back();
}

void RMGVertexConfinement::BeginOfRunAction(const G4Run*) {
  // Reset all timers and counters before the next run.
  fTrials = 0;
  fVertexGenerationTime = std::chrono::nanoseconds::zero();
}

void RMGVertexConfinement::EndOfRunAction(const G4Run* run) {
  auto n_ev = run->GetNumberOfEvent();
  if (n_ev == 0) return;
  auto avg_iter = fTrials * 1. / n_ev;
  RMGLog::OutFormat(RMGLog::summary, "Stats: on average, {:.1f} iterations were needed to sample a valid vertex ({:.1f}% efficiency)",
      avg_iter, 100 / avg_iter);
  RMGLog::OutFormat(RMGLog::summary, "Stats: average time to sample a vertex was {:.5f} us/event",
      fVertexGenerationTime.count() / n_ev / 1000.0);
}

void RMGVertexConfinement::DefineCommands() {


  fMessengers.push_back(std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/",
      "Commands for controlling primary confinement"));

  fMessengers.back()
      ->DeclareMethod("Reset", &RMGVertexConfinement::Reset)
      .SetGuidance("Reset all parameters of vertex confinement, so that it can be reconfigured.")
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.back()
      ->DeclareProperty("SampleOnSurface", fOnSurface)
      .SetGuidance("If true (or omitted argument), sample on the surface of solids")
      .SetParameterName("flag", true)
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.back()
      ->DeclareMethod("SamplingMode", &RMGVertexConfinement::SetSamplingModeString)
      .SetGuidance("Select sampling mode for volume confinement")
      .SetParameterName("mode", false)
      .SetCandidates(RMGTools::GetCandidates<SamplingMode>())
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.back()
      ->DeclareMethod("FirstSamplingVolume", &RMGVertexConfinement::SetFirstSamplingVolumeTypeString)
      .SetGuidance("Select the type of volume which will be sampled first for intersections")
      .SetParameterName("type", false)
      .SetCandidates(RMGTools::GetCandidates<VolumeType>())
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);


  fMessengers.back()
      ->DeclareProperty("MaxSamplingTrials", fMaxAttempts)
      .SetGuidance("Set maximum number of attempts for sampling primary positions in a volume")
      .SetParameterName("N", false)
      .SetRange("N > 0")
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);


  fMessengers.back()
      ->DeclareProperty("SurfaceSampleMaxIntersections", fSurfaceSampleMaxIntersections)
      .SetGuidance("Set maximum number of intersections of a line with the surface. Note: can be "
                   "set to an overestimate. ")
      .SetParameterName("N", false)
      .SetRange("N > 1")
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.back()
      ->DeclareProperty("ForceContainmentCheck", fForceContainmentCheck)
      .SetGuidance("If true (or omitted argument), perform a containment check even after sampling "
                   "from a natively sampleable object. This is only an extra sanity check that does"
                   " not alter the behaviour.")
      .SetParameterName("flag", true)
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Physical/",
          "Commands for setting physical volumes up for primary confinement"));

  fMessengers.back()
      ->DeclareMethod("AddVolume", &RMGVertexConfinement::AddPhysicalVolumeNameRegex)
      .SetGuidance("Add physical volume(s) to sample primaries from.")
      .SetParameterName(0, "regex", false, false)
      .SetParameterName(1, "copy_nr_regex", true, false)
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/",
          "Commands for setting geometrical volumes up for primary confinement"));

  fMessengers.back()
      ->DeclareMethod("AddSolid", &RMGVertexConfinement::AddGeometricalVolumeString)
      .SetGuidance("Add geometrical solid to sample primaries from")
      .SetParameterName("solid", false)
      .SetCandidates(RMGTools::GetCandidates<GeometricalSolidType>())
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);

  fMessengers.back()
      ->DeclareMethod("AddExcludedSolid", &RMGVertexConfinement::AddExcludedGeometricalVolumeString)
      .SetGuidance("Add geometrical solid to exclude samples from")
      .SetParameterName("solid", false)
      .SetCandidates(RMGTools::GetCandidates<GeometricalSolidType>())
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);


  // FIXME: see comment in .hh
  fMessengers.back()
      ->DeclareMethodWithUnit("CenterPositionX", "cm", &RMGVertexConfinement::SetGeomVolumeCenterX)
      .SetGuidance("Set center position (X coordinate)")
      .SetParameterName("value", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("CenterPositionY", "cm", &RMGVertexConfinement::SetGeomVolumeCenterY)
      .SetGuidance("Set center position (Y coordinate)")
      .SetParameterName("value", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("CenterPositionZ", "cm", &RMGVertexConfinement::SetGeomVolumeCenterZ)
      .SetGuidance("Set center position (Z coordinate)")
      .SetParameterName("value", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Sphere/",
          "Commands for setting geometrical dimensions of a sampling sphere"));

  fMessengers.back()
      ->DeclareMethodWithUnit("InnerRadius", "cm", &RMGVertexConfinement::SetGeomSphereInnerRadius)
      .SetGuidance("Set inner radius")
      .SetParameterName("L", false)
      .SetRange("L >= 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("OuterRadius", "cm", &RMGVertexConfinement::SetGeomSphereOuterRadius)
      .SetGuidance("Set outer radius")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Cylinder/",
          "Commands for setting geometrical dimensions of a sampling cylinder"));

  fMessengers.back()
      ->DeclareMethodWithUnit("InnerRadius", "cm", &RMGVertexConfinement::SetGeomCylinderInnerRadius)
      .SetGuidance("Set inner radius")
      .SetParameterName("L", false)
      .SetRange("L >= 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("OuterRadius", "cm", &RMGVertexConfinement::SetGeomCylinderOuterRadius)
      .SetGuidance("Set outer radius")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("Height", "cm", &RMGVertexConfinement::SetGeomCylinderHeight)
      .SetGuidance("Set height")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("StartingAngle", "deg",
          &RMGVertexConfinement::SetGeomCylinderStartingAngle)
      .SetGuidance("Set starting angle")
      .SetParameterName("A", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("SpanningAngle", "deg",
          &RMGVertexConfinement::SetGeomCylinderSpanningAngle)
      .SetGuidance("Set spanning angle")
      .SetParameterName("A", false)
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.push_back(
      std::make_unique<G4GenericMessenger>(this, "/RMG/Generator/Confinement/Geometrical/Box/",
          "Commands for setting geometrical dimensions of a sampling box"));

  fMessengers.back()
      ->DeclareMethodWithUnit("XLength", "cm", &RMGVertexConfinement::SetGeomBoxXLength)
      .SetGuidance("Set X length")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("YLength", "cm", &RMGVertexConfinement::SetGeomBoxYLength)
      .SetGuidance("Set Y length")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_PreInit, G4State_Idle);

  fMessengers.back()
      ->DeclareMethodWithUnit("ZLength", "cm", &RMGVertexConfinement::SetGeomBoxZLength)
      .SetGuidance("Set Z length")
      .SetParameterName("L", false)
      .SetRange("L > 0")
      .SetStates(G4State_PreInit, G4State_Idle);
}

// vim: tabstop=2 shiftwidth=2 expandtab
