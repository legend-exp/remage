#ifndef _RMG_GENERATOR_VOLUME_CONFINEMENT_HH_
#define _RMG_GENERATOR_VOLUME_CONFINEMENT_HH_

#include <vector>
#include <map>
#include <regex>

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

#include "RMGVGeneratorPrimaryPosition.hh"

class G4VSolid;
struct GenericGeometricalSolidData {
  G4String      g4_name = "";
  G4ThreeVector volume_center = G4ThreeVector(0, 0, 0);
  G4double      sphere_inner_radius = 0;
  G4double      sphere_outer_radius = -1;
  G4double      cylinder_inner_radius = 0;
  G4double      cylinder_outer_radius = -1;
  G4double      cylinder_height = -1;
  G4double      cylinder_starting_angle = 0;
  G4double      cylinder_spanning_angle = CLHEP::twopi;
  G4double      box_x_length = -1;
  G4double      box_y_length = -1;
  G4double      box_z_length = -1;
};

class G4VPhysicalVolume;
class G4VSolid;
class RMGGeneratorVolumeConfinementMessenger;
class RMGGeneratorVolumeConfinement : public RMGVGeneratorPrimaryPosition {

  public:

    enum SamplingMode {
      kIntersectWithGeometrical,
      kUnionAll
    };

    RMGGeneratorVolumeConfinement();
    ~RMGGeneratorVolumeConfinement();

    G4ThreeVector ShootPrimaryPosition() override;

    // to be used in the messenger class
    inline void AddPhysicalVolumeNameRegex(G4String& name, G4String copy_nr=".*") {
      fPhysicalVolumeNameRegexes.emplace_back(name);
      fPhysicalVolumeCopyNrRegexes.emplace_back(copy_nr);
    }

    inline void AddGeometricalVolume(GenericGeometricalSolidData& data) { fGeomVolumeData.emplace_back(data); }

    inline void SetSamplingMode(SamplingMode mode) { fSamplingMode = mode; }

    inline std::vector<GenericGeometricalSolidData>& GetGeometricalSolidDataList();

  private:

    void ParsePhysicalVolumeInfoRegex();
    void InitializeGeometricalVolumes();
    void InitializeSamplingVolume();

    std::vector<G4String> fPhysicalVolumeNameRegexes;
    std::vector<G4String> fPhysicalVolumeCopyNrRegexes;

    std::map<G4VPhysicalVolume*, G4double> fPhysicalVolumes;

    SamplingMode fSamplingMode;

    std::vector<GenericGeometricalSolidData> fGeomVolumeData;
    std::vector<G4VSolid*> fGeomVolumeSolids;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
