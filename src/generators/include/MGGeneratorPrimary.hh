#ifndef _MGGENERATORPRIMARY_HH
#define _MGGENERATORPRIMARY_HH

#include <vector>

#include "Rtypes.h"
#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"

class TChain;
class MGVGenerator;
class MGGeneratorPrimaryMessenger;
class MGGeneratorPositionSampling;
class MGGeneratorPositionSamplingGeometrical;
class MGGeneratorPositionSamplingDetectorShell;
class MGGeneratorPrimary : public G4VUserPrimaryGeneratorAction {

  public:

    enum EConfinementCode {
      noconfined,
      volume,
      volumelist,
      volumearray,
      surface,
      surfacelist,
      geometricalvolume,
      geometricalsurface,
      detectorshell
    };

    MGGeneratorPrimary();
    ~MGGeneratorPrimary();

    void GeneratePrimaries(G4Event *event);

    MGVGenerator*    GetMGGenerator()            { return fMGGenerator;            }
    G4String         GetVolumeName()             { return fVolumeName;             }
    EConfinementCode GetConfinementCode()        { return fConfinementCode;        }
    G4String         GetVolumeListName()         { return fVolumeListName;         }
    G4int            GetVolumeListFrom()         { return fVolumeListFrom;         }
    G4int            GetVolumeListTo()           { return fVolumeListTo;           }
    G4bool           GetVolumeListInitialized()  { return fVolumeListInitialized;  }
    G4bool           GetVolumeArrayInitialized() { return fVolumeArrayInitialized; }
    G4ThreeVector    GetParticlePosition()       { return fPosition;               }

    void SetConfinementCode(EConfinementCode code);
    void SetMGGenerator(MGVGenerator* gene)         { fMGGenerator = gene;              }
    void SetVolumeName(G4String name)               { fVolumeName = name;               }
    void SetVolumeListName(G4String name)           { fVolumeListName = name;           }
    void SetVolumeListFrom(G4int n)                 { fVolumeListFrom = n;              }
    void SetVolumeListTo(G4int n)                   { fVolumeListTo = n;                }
    void SetVolumeListInitialized(G4bool val)       { fVolumeListInitialized = val;     }
    void SetVolumeArrayInitialized(G4bool val)      { fVolumeArrayInitialized = val;    }
    void SetParticlePosition(G4ThreeVector vec)     { fPosition = vec;                  }
    void SetGSSOffset(G4double GSSOffset)           { fGSSOffset = GSSOffset;           }
    void SetGSSEventNumber(Long64_t GSSEventNumber) { fGSSEventNumber = GSSEventNumber; }

    void AddVolumeNumberToList(G4int nnn);
    void AddVolumeNameToArray(G4String pname);
    void AddGSSFile(const char* filename);
    void ClearList();

  private:

    void InitializeVolumeListSampling();
    void InitializeVolumeArraySampling();
    G4String ChooseVolumeFromList();
    G4String ChooseVolumeFromArray();

    EConfinementCode                          fConfinementCode;
    MGVGenerator*                             fMGGenerator;
    MGGeneratorPrimaryMessenger*              fG4Messenger;
    G4ThreeVector                             fPosition;
    MGGeneratorPositionSampling*              fPositionSampler;
    MGGeneratorPositionSamplingGeometrical*   fPositionSamplerGeometrical;
    MGGeneratorPositionSamplingDetectorShell* fPositionSamplerDetectorShell;
    G4String                                  fVolumeName;
    G4String                                  fVolumeListName;
    G4int                                     fVolumeListFrom;
    G4int                                     fVolumeListTo;
    G4bool                                    fVolumeListInitialized;
    G4bool                                    fVolumeArrayInitialized;
    std::vector<G4double>                     fMassFractionForVolumeList;
    std::vector<G4int>                        fIDVolumeList;
    std::vector<G4double>                     fMassFractionForVolumeArray;
    std::vector<G4String>                     fIDVolumeArray;
    G4double                                  fGSSOffset;
    Long64_t                                  fGSSEventNumber;
    TChain*                                   fGSSChain;
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
