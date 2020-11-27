#ifndef _MGGENERATORPRIMARY_HH
#define _MGGENERATORPRIMARY_HH

#include <vector>

#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ThreeVector.hh"

class MGVGenerator;
class MGGeneratorPrimaryMessenger;
class MGGeneratorPositionSampling;
class MGGeneratorPrimary : public G4VUserPrimaryGeneratorAction {

  public:

    enum EConfinementCode {
      noconfined,
      volume,
      volumelist,
      volumearray
    };

    MGGeneratorPrimary();
    ~MGGeneratorPrimary();

    MGGeneratorPrimary           (MGGeneratorPrimary const&) = delete;
    MGGeneratorPrimary& operator=(MGGeneratorPrimary const&) = delete;
    MGGeneratorPrimary           (MGGeneratorPrimary&&)      = delete;
    MGGeneratorPrimary& operator=(MGGeneratorPrimary&&)      = delete;

    void GeneratePrimaries(G4Event *event) override;

    inline MGVGenerator*    GetMGGenerator()            { return fMGGenerator;            }
    inline G4String         GetVolumeName()             { return fVolumeName;             }
    inline EConfinementCode GetConfinementCode()        { return fConfinementCode;        }
    inline G4String         GetVolumeListName()         { return fVolumeListName;         }
    inline G4int            GetVolumeListFrom()         { return fVolumeListFrom;         }
    inline G4int            GetVolumeListTo()           { return fVolumeListTo;           }
    inline G4bool           GetVolumeListInitialized()  { return fVolumeListInitialized;  }
    inline G4bool           GetVolumeArrayInitialized() { return fVolumeArrayInitialized; }
    inline G4ThreeVector    GetParticlePosition()       { return fPosition;               }

    void SetConfinementCode(EConfinementCode code);
    inline void SetMGGenerator(MGVGenerator* gene)         { fMGGenerator = gene;              }
    inline void SetVolumeName(G4String name)               { fVolumeName = name;               }
    inline void SetVolumeListName(G4String name)           { fVolumeListName = name;           }
    inline void SetVolumeListFrom(G4int n)                 { fVolumeListFrom = n;              }
    inline void SetVolumeListTo(G4int n)                   { fVolumeListTo = n;                }
    inline void SetVolumeListInitialized(G4bool val)       { fVolumeListInitialized = val;     }
    inline void SetVolumeArrayInitialized(G4bool val)      { fVolumeArrayInitialized = val;    }
    inline void SetParticlePosition(G4ThreeVector& vec)    { fPosition = vec;                  }

    void AddVolumeNumberToList(G4int nnn);
    void AddVolumeNameToArray(G4String pname);
    inline void ClearList() { fIDVolumeList.clear(); }

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
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
