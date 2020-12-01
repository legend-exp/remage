#ifndef RMGGENERATORPOSITIONSAMPLING_HH_
#define RMGGENERATORPOSITIONSAMPLING_HH_

#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

class G4Navigator;
class G4VPhysicalVolume;
class G4VSolid;
class G4GeometryType;
class RMGGeneratorPositionSamplingMessenger;
class RMGGeneratorPositionSampling {

  public:

    RMGGeneratorPositionSampling();
    ~RMGGeneratorPositionSampling();

    RMGGeneratorPositionSampling           (RMGGeneratorPositionSampling const&) = delete;
    RMGGeneratorPositionSampling& operator=(RMGGeneratorPositionSampling const&) = delete;
    RMGGeneratorPositionSampling           (RMGGeneratorPositionSampling&&)      = delete;
    RMGGeneratorPositionSampling& operator=(RMGGeneratorPositionSampling&&)      = delete;

    G4ThreeVector SampleUniformlyInVolume(G4String volume_name, G4int copy_nr);
    G4ThreeVector SampleOnSurface(G4String volume_name, G4int copy_nr);
    G4ThreeVector SampleInGeometricalVolume();
    G4ThreeVector SampleInGeometricalSurface();

    inline void SetGlobalTranslation(G4ThreeVector vect)  { fVolumeGlobalTranslation = vect; }
    inline void SetGlobalRotation(G4RotationMatrix mat)   { fVolumeGlobalRotation = mat; }
    inline void SetGeometricalVolumeName(G4String name)   { fGeometricalVolumeName = name; }
    inline void SetGeometricalSurfaceName(G4String name)  { fGeometricalSurfaceName = name; }
    inline void SetCenterCoordinates(G4ThreeVector coord) { fCenterCoordinates = coord;}
    inline void SetInnerSphereRadius(G4double isr)        { fInnerSphereRadius = isr; }
    inline void SetOuterSphereRadius(G4double osr)        { fOuterSphereRadius = osr; }
    inline void SetInnerDiskRadius(G4double idr)          { fInnerDiskRadius = idr; }
    inline void SetOuterDiskRadius(G4double odr)          { fOuterDiskRadius = odr; }
    inline void SetInnerCylinderRadius(G4double icr)      { fInnerCylinderRadius = icr; }
    inline void SetOuterCylinderRadius(G4double ocr)      { fOuterCylinderRadius = ocr; }
    inline void SetCylinderHeight(G4double cylh)          { fCylinderHeight = cylh; }
    inline void SetCylinderStartingAngle(G4double san)    { fCylinderStartingAngle = san; }
    inline void SetCylinderSpanningAngle(G4double span)   { fCylinderSpanningAngle = span; }
    inline void SetXLengthBox(G4double xlb)               { fXLengthBox = xlb; }
    inline void SetYLengthBox(G4double ylb)               { fYLengthBox = ylb; }
    inline void SetZLengthBox(G4double zlb)               { fZLengthBox = zlb; }
    inline void SetXCenter(G4double xcenter)              { fXCenter = xcenter; fUseCenterManually = true; }
    inline void SetYCenter(G4double ycenter)              { fYCenter = ycenter; fUseCenterManually = true; }
    inline void SetZCenter(G4double zcenter)              { fZCenter = zcenter; fUseCenterManually = true; }
    inline void SetUseCenterManually(G4bool val)          { fUseCenterManually = val; }
    inline void SetNPositionsamplingMax(const G4int val)  { fNPositionsamplingMax = val; }

    inline G4bool GetUseCenterManually()              { return fUseCenterManually; }
    inline G4int GetNPositionsamplingMax() const      { return fNPositionsamplingMax; }
    inline G4String GetGeometricalVolumeName() const  { return fGeometricalVolumeName; }
    inline G4String GetGeometricalSurfaceName() const { return fGeometricalSurfaceName; }
    inline G4ThreeVector GetGlobalTranslation() const { return fVolumeGlobalTranslation; }
    inline G4RotationMatrix GetGlobalRotation() const { return fVolumeGlobalRotation; }

    inline G4VSolid* GetGeometricalSamplingShape() { return fGeomSamplingShape; }

  private:

    G4bool InitializeSamplingVolume();
    G4VPhysicalVolume* FindTheDirectMother(G4VPhysicalVolume* volume);

    RMGGeneratorPositionSamplingMessenger* fMessenger;

    G4String fVolumeName;
    G4int fVolumeCopyNumber;
    G4VPhysicalVolume* fVolumePhysical;
    G4RotationMatrix fVolumeGlobalRotation;
    G4ThreeVector fVolumeGlobalTranslation;

    G4Navigator* fGeomNavigator;

    G4double fRadius;
    G4ThreeVector fCenter;

    G4GeometryType fSolidType;
    G4bool fHasDaughters;

    G4GeometryType fSolidDaughter1_type;
    G4GeometryType fSolidDaughter2_type;

    G4int fNTries;
    G4int fNCalls;

    // Messenger stuff

    G4String fGeometricalSurfaceName;
    G4String fGeometricalVolumeName;

    G4ThreeVector fCenterCoordinates;

    // geometrical parameters
    G4double fInnerSphereRadius;
    G4double fOuterSphereRadius;
    G4double fInnerDiskRadius;
    G4double fOuterDiskRadius;
    G4double fInnerCylinderRadius;
    G4double fOuterCylinderRadius;
    G4double fCylinderHeight;
    G4double fCylinderStartingAngle;
    G4double fCylinderSpanningAngle;
    G4double fXLengthBox;
    G4double fYLengthBox;
    G4double fZLengthBox;
    G4double fXCenter;
    G4double fYCenter;
    G4double fZCenter;
    G4bool   fUseCenterManually;

    G4VSolid* fGeomSamplingShape;

    // size of the World Volume Box (used for the dummy vertex)
    G4double fWorldVolumeHalfLength;
    G4int    fNPositionsamplingMax;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
