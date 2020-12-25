#ifndef _RMG_GENERATOR_VOLUME_CONFINEMENT_HH_
#define _RMG_GENERATOR_VOLUME_CONFINEMENT_HH_

#include <vector>
#include <regex>

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"

#include "RMGVGeneratorPrimaryPosition.hh"

class G4VPhysicalVolume;
class G4VSolid;
class RMGGeneratorVolumeConfinementMessenger;
class RMGGeneratorVolumeConfinement : public RMGVGeneratorPrimaryPosition {

  public:

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

    enum SamplingMode {
      kIntersectPhysicalWithGeometrical,
      kUnionAll
    };

    RMGGeneratorVolumeConfinement();
    ~RMGGeneratorVolumeConfinement();

    G4ThreeVector ShootPrimaryPosition() override;

    // to be used in the messenger class
    inline void AddPhysicalVolumeNameRegex(G4String name, G4String copy_nr=".*") {
      fPhysicalVolumeNameRegexes.emplace_back(name);
      fPhysicalVolumeCopyNrRegexes.emplace_back(copy_nr);
    }

    inline void AddGeometricalVolume(GenericGeometricalSolidData& data) { fGeomVolumeData.emplace_back(data); }
    void Reset();

    inline void SetSamplingMode(SamplingMode mode) { fSamplingMode = mode; }
    inline void SetBoundingSolidType(G4String type) { fBoundingSolidType = type; }

    inline std::vector<GenericGeometricalSolidData>& GetGeometricalSolidDataList() { return fGeomVolumeData; }

    struct SampleableObject {

      SampleableObject() = default;
      SampleableObject(G4VPhysicalVolume* v, G4RotationMatrix r, G4ThreeVector t, G4VSolid* s);
      ~SampleableObject();

      G4VPhysicalVolume* physical_volume;
      G4VSolid*          sampling_solid;
      G4RotationMatrix   rotation;
      G4ThreeVector      translation;
      G4double           volume;
      G4double           surface;
      G4bool             containment_check;
    };

    struct SampleableObjectCollection {

      inline SampleableObjectCollection() : total_volume(0), total_surface(0) {}
      inline ~SampleableObjectCollection() { data.clear(); }

      const SampleableObject& SurfaceWeightedRand();
      const SampleableObject& VolumeWeightedRand();
      G4bool IsInside(const G4ThreeVector& point);

      // emulate std::vector
      void emplace_back(G4VPhysicalVolume* v, G4RotationMatrix& r, G4ThreeVector& t, G4VSolid* s);
      void emplace_back(G4VPhysicalVolume* v, G4RotationMatrix r, G4ThreeVector t, G4VSolid* s);
      inline G4bool empty() { return data.empty(); }
      inline SampleableObject& back() { return data.back(); }
      inline void clear() { data.clear(); }

      std::vector<SampleableObject> data;
      G4double total_volume;
      G4double total_surface;
    };

  private:

    void InitializePhysicalVolumes();
    void InitializeGeometricalVolumes();

    std::vector<G4String> fPhysicalVolumeNameRegexes;
    std::vector<G4String> fPhysicalVolumeCopyNrRegexes;

    std::vector<GenericGeometricalSolidData> fGeomVolumeData;
    SampleableObjectCollection fPhysicalVolumes;
    SampleableObjectCollection fGeomVolumeSolids;

    SamplingMode fSamplingMode;
    G4bool       fOnSurface;
    G4String     fBoundingSolidType;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
