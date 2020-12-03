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

#include "RMGGeneratorVolumeConfinementMessenger.hh"
#include "RMGGeneratorUtil.hh"
#include "RMGLog.hh"
// #include "RMGPhysVolNavigator.hh"
#include "RMGManager.hh"

RMGGeneratorVolumeConfinement::RMGGeneratorVolumeConfinement() :
  RMGVGeneratorPrimaryPosition("VolumeConfinement"),
  fSamplingMode(SamplingMode::kUnionAll) {

  // fGeomNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  fG4Messenger = new RMGGeneratorVolumeConfinementMessenger(this) ;
}

// RMGGeneratorVolumeConfinement::~RMGGeneratorVolumeConfinement() {
//   delete fMessenger;
// }

void RMGGeneratorVolumeConfinement::ParsePhysicalVolumeInfoRegex() {

  auto volume_store = G4PhysicalVolumeStore::GetInstance();

  G4double tot_volume = 0;
  // scan all search patterns provided by the user
  for (size_t i = 0; i < fPhysicalVolumeNameRegexes.size(); ++i) {
    RMGLog::OutFormat(RMGLog::detail, "Physical volumes matching pattern '%s'['%s']",
        fPhysicalVolumeNameRegexes.at(i).c_str(), fPhysicalVolumeCopyNrRegexes.at(i).c_str());

    G4bool found = false;
    // scan the volume store for matches
    for (auto&& it = volume_store->begin(); it != volume_store->end(); it++) {
      if (std::regex_match((*it)->GetName(), std::regex(fPhysicalVolumeNameRegexes.at(i))) and
          std::regex_match(std::to_string((*it)->GetCopyNo()), std::regex(fPhysicalVolumeCopyNrRegexes.at(i)))) {

        fPhysicalVolumes.emplace(*it, (*it)->GetLogicalVolume()->GetSolid()->GetCubicVolume());
        tot_volume += fPhysicalVolumes[*it];

        RMGLog::OutFormat(RMGLog::detail, "Mass of '%s[%s]' = %g kg", (*it)->GetName().c_str(),
            (*it)->GetCopyNo(), fPhysicalVolumes[*it]/CLHEP::kg);

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

  // normalize vector
  for (auto&& m : fPhysicalVolumes) m.second /= tot_volume;
}

void RMGGeneratorVolumeConfinement::InitializeGeometricalVolumes() {

  for (const auto& d : fGeomVolumeData) {
    if (d.g4_name == "Sphere" or d.g4_name == "SphericalShell") {
      fGeomVolumeSolids.push_back(new G4Sphere("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
          d.sphere_inner_radius, d.sphere_outer_radius, 0, CLHEP::twopi, 0, CLHEP::pi));
    }
    else if (d.g4_name == "Cylinder" or d.g4_name == "CylindricalShell") {
      fGeomVolumeSolids.push_back(new G4Tubs("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
          d.cylinder_inner_radius, d.cylinder_outer_radius, 0.5*d.cylinder_height,
          d.cylinder_starting_angle, d.cylinder_spanning_angle));
    }
    else if (d.g4_name == "Box") {
      fGeomVolumeSolids.push_back(new G4Box("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
          0.5*d.box_x_length, 0.5*d.box_y_length, 0.5*d.box_z_length));
    }
    else {
      RMGLog::Out(RMGLog::error, "Geometrical solid '", d.g4_name, "' not known! (Implement me?)");
    }
  }
}

void RMGGeneratorVolumeConfinement::InitializeSamplingVolume() {

  this->ParsePhysicalVolumeInfoRegex();
  this->InitializeGeometricalVolumes();

  // auto the_solid = fVolumePhysical->GetLogicalVolume()->GetSolid();
  // fHasDaughters = !fVolumePhysical->GetLogicalVolume()->GetNoDaughters();
  // fSolidType = the_solid->GetEntityType();

  // // if a geometrical volume is set, save its size
  // if (fGeometricalVolumeName == "Sphere" or fGeometricalVolumeName == "SphericalShell") {
  //   fGeomSamplingShape = new G4Sphere("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
  //       fInnerSphereRadius, fOuterSphereRadius, 0, CLHEP::twopi, 0, CLHEP::pi);
  // }
  // else if (fGeometricalVolumeName == "Cylinder" or fGeometricalVolumeName == "CylindricalShell") {
  //   fGeomSamplingShape = new G4Tubs("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
  //       fInnerCylinderRadius, fOuterCylinderRadius, 0.5*fCylinderHeight, 0, CLHEP::twopi);
  // }
  // else if (fGeometricalVolumeName == "Box") {
  //   fGeomSamplingShape = new G4Box("RMGGeneratorVolumeConfinement::fGeomSamplingShape",
  //       0.5*fXLengthBox, 0.5*fYLengthBox, 0.5*fZLengthBox);
  // }
  // // is not set, see what kind of physical volume do we have
  // else if (fGeometricalVolumeName.empty()) {
  //   if (fSolidType == "G4Box") {
  //     fGeomSamplingShape = dynamic_cast<G4Box*>(the_solid);
  //   }
  //   else if (fSolidType == "G4Tubs") {
  //     fGeomSamplingShape = dynamic_cast<G4Tubs*>(the_solid);
  //   }
  //   else if (fSolidType == "G4Sphere") {
  //     fGeomSamplingShape = dynamic_cast<G4Sphere*>(the_solid);
  //   }
  //   else if (fSolidType == "G4Orb") {
  //     fGeomSamplingShape = dynamic_cast<G4Orb*>(the_solid);
  //   }
  //   // TODO: this needs a serious review
  //   else if (fSolidType == "G4SubtractionSolid") {
  //     G4VSolid* firstSolid = (G4SubtractionSolid*)the_solid->GetConstituentSolid(0);
  //     fSolidDaughter1_type = firstSolid->GetEntityType();
  //     G4VSolid* secondSolid = (G4SubtractionSolid*)the_solid->GetConstituentSolid(1);
  //     fSolidDaughter2_type = secondSolid->GetEntityType();
  //     if (fSolidDaughter1_type == "G4Tubs") {
  //       fSolid_par_arr[0] = ((G4Tubs*)firstSolid)->GetInnerRadius();
  //       fSolid_par_arr[1] = ((G4Tubs*)firstSolid)->GetOuterRadius();
  //       fSolid_par_arr[2] = ((G4Tubs*)firstSolid)->GetZHalfLength();
  //       fSolid_par_arr[3] = ((G4Tubs*)firstSolid)->GetStartPhiAngle();
  //       fSolid_par_arr[4] = ((G4Tubs*)firstSolid)->GetDeltaPhiAngle();
  //       fHasDaughters = true; // this means that the first volume has an "hole"
  //     }
  //     else { // two different bounding volumes available, Sphere or Box. Use the one with smaller volume, to speed up sampling
  //       G4double x_half_length = the_solid->GetExtent().GetXmax();
  //       G4double y_half_length = the_solid->GetExtent().GetYmax();
  //       G4double z_half_length = the_solid->GetExtent().GetZmax();
  //       fSolid_par_arr[0] = -x_half_length;
  //       fSolid_par_arr[1] =  x_half_length;
  //       fSolid_par_arr[2] = -y_half_length;
  //       fSolid_par_arr[3] =  y_half_length;
  //       fSolid_par_arr[4] = -z_half_length;
  //       fSolid_par_arr[5] =  z_half_length;
  //       fRadius = the_solid->GetExtent().GetExtentRadius();

  //       G4double vol_boundsphere = 4./3 * M_PI * fRadius * fRadius * fRadius;
  //       G4double vol_boundbox    = (x_half_length * 2) * (y_half_length * 2) * (z_half_length * 2);

  //       fSolidType = "BoundingSphere";
  //       if( getenv("UseBoundingBox")!= nullptr  ) {
  //         G4String strUseBoundingBox;
  //         strUseBoundingBox = getenv("UseBoundingBox");
  //         strUseBoundingBox.toLower();
  //         if( strUseBoundingBox=="yes" ) {
  //           std::cout << "Enable use of Bounding Box for position sampling." << std::endl;
  //           if(vol_boundsphere <= vol_boundbox) fSolidType = "BoundingSphere";
  //           else                                fSolidType = "BoundingBox";
  //         }
  //       }
  //     }
  //   }
  // }
  // else {
  //   // two different bounding volumes available. Use the one with smaller volume, to speed up sampling
  //   G4double x_half_length = the_solid->GetExtent().GetXmax();
  //   G4double y_half_length = the_solid->GetExtent().GetYmax();
  //   G4double z_half_length = the_solid->GetExtent().GetZmax();
  //   fSolid_par_arr[0] = -x_half_length;
  //   fSolid_par_arr[1] =  x_half_length;
  //   fSolid_par_arr[2] = -y_half_length;
  //   fSolid_par_arr[3] =  y_half_length;
  //   fSolid_par_arr[4] = -z_half_length;
  //   fSolid_par_arr[5] =  z_half_length;
  //   fRadius = the_solid->GetExtent().GetExtentRadius();

  //   G4double vol_boundsphere = 4./3 * 3.1415 * fRadius * fRadius * fRadius;
  //   G4double vol_boundbox    = ( x_half_length * 2) * ( y_half_length * 2) * ( z_half_length * 2);

  //   fSolidType = "BoundingSphere";
  //   if( getenv("UseBoundingBox")!= nullptr ) {
  //     G4String strUseBoundingBox;
  //     strUseBoundingBox = getenv("UseBoundingBox");
  //     strUseBoundingBox.toLower();
  //     if( strUseBoundingBox == "yes" ) {
  //       std::cout << "Enable use of Bounding Box for position sampling." << std::endl;
  //       if(vol_boundsphere <= vol_boundbox) fSolidType = "BoundingSphere";
  //       else                                fSolidType = "BoundingBox";
  //     }
  //   }
  // }

  // fRadius = the_solid->GetExtent().GetExtentRadius();
  // auto center_point = the_solid->GetExtent().GetExtentCenter();
  // fCenter = G4ThreeVector(center_point.x(), center_point.y(), center_point.z());
  // // RMGLog(debugging) << "Sphere for solid. " << "Radius " << fRadius/mm << " mm"
  // //                  << " centered at (" << center_point.x() << ","
  // //                  << center_point.y() << "," << center_point.z() << ")");

  // auto world_volume = fGeomNavigator->GetWorldVolume();
  // if (!world_volume) RMGLog::Out(RMGLog::fatal, "World volume not defined");

  // // start from selected volume
  // auto ivolume = fVolumePhysical;
  // G4ThreeVector origin(0, 0, 0);
  // G4RotationMatrix identity;
  // G4ThreeVector vol_global_translation = origin;
  // G4RotationMatrix vol_global_rotation = identity;   // Initialised to identity

  // std::vector<G4RotationMatrix> partial_rotations;
  // std::vector<G4ThreeVector> partial_translations;

  // for (G4int i = 0; i < n_volumes and ivolume != world_volume; i++) {
  //   partial_rotations.push_back((*(ivolume->GetObjectRotation())));
  //   partial_translations.push_back((ivolume->GetObjectTranslation()));
  //   vol_global_rotation = (*(ivolume->GetObjectRotation())) * vol_global_rotation;
  //   ivolume = RMGPhysVolNavigator::FindDirectMother(ivolume);
  //   if (ivolume == nullptr) {
  //     // TODO: i guess this is not good, what to do here?
  //     return false;
  //   }
  // }
  // // This is the world volume, which is not included in the previous loop
  // partial_rotations.push_back(identity);
  // partial_translations.push_back(origin);

  // /*
  //   partial_rotations[0] and partial_translations[0] refer to the target volume
  //   partial_rotations[1] and partial_translations[1], to the direct ancestor, etc.
  //   It is necessary to rotate with respect to the frame of the ancestor.
  //   The loop should be "transparent" (namely, should work as the previous version)
  //   if there are no rotations (or only the target volume is rotated): rotations are
  //   identity matrices and vol_global_translation = sum(partial_translations)
  // */
  // if (partial_rotations.size() > 1) { // otherwise the target is the World!
  //   for (size_t i = 0; i < partial_rotations.size()-1; i++) {
  //     auto tmp_rot = identity;
  //     for (size_t j = i+1; j < partial_rotations.size()-1; j++) {
  //       tmp_rot = partial_rotations[j] * tmp_rot;
  //     }
  //     vol_global_translation += tmp_rot * partial_translations[i];
  //   }
  // }
  // else RMGLog::Out(RMGLog::warning, "Target = world (?)");

  // this->SetGlobalRotation(vol_global_rotation);

  // // add offset
  // if (fUseCenterManually) {
  //   G4ThreeVector corrected_transl = vol_global_translation;
  //   corrected_transl.setX(fXCenter + vol_global_translation.x() );
  //   corrected_transl.setY(fYCenter + vol_global_translation.y() );
  //   corrected_transl.setZ(fZCenter + vol_global_translation.z() );
  //   this->SetGlobalTranslation(corrected_transl);
  // }

  // return is_volume_found;
}

G4ThreeVector RMGGeneratorVolumeConfinement::ShootPrimaryPosition() {

  switch (fSamplingMode) {
    case SamplingMode::kIntersectWithGeometrical :
      break;
    case SamplingMode::kUnionAll :
      break;
  }
}

// vim: tabstop=2 shiftwidth=2 expandtab
