#ifndef _RMG_GENERATOR_FROM_FILE_HH_
#define _RMG_GENERATOR_FROM_FILE_HH_

#include <memory>
#include <string>

#include "RMGVVertexGenerator.hh"

#include "G4GenericMessenger.hh"
#include "G4ThreeVector.hh"

class G4Event;
class G4VAnalysisReader;
class RMGVertexFromFile : public RMGVVertexGenerator {

  public:

    RMGVertexFromFile();
    inline ~RMGVertexFromFile() = default;

    RMGVertexFromFile(RMGVertexFromFile const&) = delete;
    RMGVertexFromFile& operator=(RMGVertexFromFile const&) = delete;
    RMGVertexFromFile(RMGVertexFromFile&&) = delete;
    RMGVertexFromFile& operator=(RMGVertexFromFile&&) = delete;

    bool GeneratePrimariesVertex(G4ThreeVector&) override;

    void OpenFile(std::string& name);

  private:

    G4VAnalysisReader* fReader = nullptr;
    std::string fFileName;

    std::unique_ptr<G4GenericMessenger> fMessenger = nullptr;
    void DefineCommands();
};

#endif

// vim: tabstop=2 shiftwidth=2 expandtab
