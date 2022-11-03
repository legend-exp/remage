#ifndef _RMG_VERTEX_CONFINEMENT_HH_
#define _RMG_VERTEX_CONFINEMENT_HH_

#include <regex>
#include <vector>

#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4UnitsTable.hh"

#include "RMGVVertexGenerator.hh"

class G4VPhysicalVolume;
class G4VSolid;
class G4GenericMessenger;
class RMGVertexConfinement : public RMGVVertexGenerator {

  public:

    struct GenericGeometricalSolidData {
        std::string g4_name = "";
        G4ThreeVector volume_center = G4ThreeVector(0, 0, 0);
        double sphere_inner_radius = 0;
        double sphere_outer_radius = -1;
        double cylinder_inner_radius = 0;
        double cylinder_outer_radius = -1;
        double cylinder_height = -1;
        double cylinder_starting_angle = 0;
        double cylinder_spanning_angle = CLHEP::twopi;
        double box_x_length = -1;
        double box_y_length = -1;
        double box_z_length = -1;
    };

    enum SamplingMode {
      kIntersectPhysicalWithGeometrical,
      kUnionAll
    };

    RMGVertexConfinement();

    void GeneratePrimariesVertex(G4ThreeVector& v) override;

    // to be used in the messenger class
    inline void AddPhysicalVolumeNameRegex(std::string name, std::string copy_nr = ".*") {
      fPhysicalVolumeNameRegexes.emplace_back(name);
      fPhysicalVolumeCopyNrRegexes.emplace_back(copy_nr);
    }
    void AddPhysicalVolumeString(std::string expr);

    inline void AddGeometricalVolume(GenericGeometricalSolidData& data) {
      fGeomVolumeData.emplace_back(data);
    }
    void Reset();

    inline void SetSamplingMode(SamplingMode mode) { fSamplingMode = mode; }
    inline void SetBoundingSolidType(std::string type) { fBoundingSolidType = type; }

    inline std::vector<GenericGeometricalSolidData>& GetGeometricalSolidDataList() {
      return fGeomVolumeData;
    }

    struct SampleableObject {

        SampleableObject() = default;
        SampleableObject(G4VPhysicalVolume* v, G4RotationMatrix r, G4ThreeVector t, G4VSolid* s);
        // NOTE: G4 volume/solid pointers should be fully owned by G4, avoid trying to delete them
        ~SampleableObject() = default;

        G4VPhysicalVolume* physical_volume = nullptr;
        G4VSolid* sampling_solid = nullptr;
        G4RotationMatrix rotation;
        G4ThreeVector translation;
        double volume = -1;
        double surface = -1;
        bool containment_check = true;
    };

    struct SampleableObjectCollection {

        SampleableObjectCollection() = default;
        inline ~SampleableObjectCollection() { data.clear(); }

        const SampleableObject& SurfaceWeightedRand() const;
        const SampleableObject& VolumeWeightedRand() const;
        bool IsInside(const G4ThreeVector& point) const;

        // emulate std::vector
        size_t size() const { return data.size(); }
        SampleableObject& at(size_t i) { return data.at(i); }
        void emplace_back(G4VPhysicalVolume* v, const G4RotationMatrix& r, const G4ThreeVector& t,
            G4VSolid* s);
        inline void push_back(const SampleableObject& obj) {
          this->emplace_back(obj.physical_volume, obj.rotation, obj.translation, obj.sampling_solid);
        }
        inline bool empty() const { return data.empty(); }
        inline SampleableObject& back() { return data.back(); }
        inline void clear() { data.clear(); }
        inline void insert(SampleableObjectCollection& other) {
          for (size_t i = 0; i < other.size(); ++i) this->push_back(other.at(i));
        }

        std::vector<SampleableObject> data;
        double total_volume = 0;
        double total_surface = 0;
    };

  private:

    void InitializePhysicalVolumes();
    void InitializeGeometricalVolumes();

    std::vector<std::string> fPhysicalVolumeNameRegexes;
    std::vector<std::string> fPhysicalVolumeCopyNrRegexes;

    std::vector<GenericGeometricalSolidData> fGeomVolumeData;
    SampleableObjectCollection fPhysicalVolumes;
    SampleableObjectCollection fGeomVolumeSolids;

    SamplingMode fSamplingMode = kUnionAll;
    bool fOnSurface = false;
    std::string fBoundingSolidType = "Sphere";

    std::vector<std::unique_ptr<G4GenericMessenger>> fMessengers;
    void SetSamplingModeString(std::string mode);
    void AddGeometricalVolumeString(std::string solid);
    GenericGeometricalSolidData& SafeBack();

    // FIXME: there is no easy way to set the position vector all at once with
    // G4GenericMessenger. Only ::DeclarePropertyWithUnit() accepts vectors and
    // one cannot call functions with more than 2 arguments (3 coordinated + 1
    // units needed). Ugly!
    inline void SetGeomVolumeCenter(const G4ThreeVector& v) { this->SafeBack().volume_center = v; }
    inline void SetGeomVolumeCenterX(double x) { this->SafeBack().volume_center.setX(x); }
    inline void SetGeomVolumeCenterY(double y) { this->SafeBack().volume_center.setY(y); }
    inline void SetGeomVolumeCenterZ(double z) { this->SafeBack().volume_center.setZ(z); }

    inline void SetGeomSphereInnerRadius(double r) { this->SafeBack().sphere_inner_radius = r; }
    inline void SetGeomSphereOuterRadius(double r) { this->SafeBack().sphere_outer_radius = r; }

    inline void SetGeomCylinderInnerRadius(double r) { this->SafeBack().cylinder_inner_radius = r; }
    inline void SetGeomCylinderOuterRadius(double r) { this->SafeBack().cylinder_outer_radius = r; }
    inline void SetGeomCylinderHeight(double h) { this->SafeBack().cylinder_height = h; }
    inline void SetGeomCylinderStartingAngle(double a) {
      this->SafeBack().cylinder_starting_angle = a;
    }
    inline void SetGeomCylinderSpanningAngle(double a) {
      this->SafeBack().cylinder_spanning_angle = a;
    }

    inline void SetGeomBoxXLength(double x) { this->SafeBack().box_x_length = x; }
    inline void SetGeomBoxYLength(double y) { this->SafeBack().box_y_length = y; }
    inline void SetGeomBoxZLength(double z) { this->SafeBack().box_z_length = z; }

    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
