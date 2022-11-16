#include "RMGVertexFromFile.hh"

#include "G4VAnalysisReader.hh"
#include "G4AnalysisUtilities.hh"
#include "G4RootAnalysisReader.hh"
#ifdef TOOLS_USE_HDF5
#include "G4Hdf5AnalysisReader.hh"
#endif
#include "G4CsvAnalysisReader.hh"
#include "G4XmlAnalysisReader.hh"

#include "RMGLog.hh"

RMGVertexFromFile::RMGVertexFromFile()
   : RMGVVertexGenerator("FromFile") {

    this->DefineCommands();
}

void RMGVertexFromFile::OpenFile(std::string& name) {
    // NOTE: GetExtension() returns a default extension if there is no file extension
    auto ext = G4Analysis::GetExtension(name);
    if (ext == "root") fReader = G4RootAnalysisReader::Instance();
    else if (ext == "hdf5") {
#ifdef TOOLS_USE_HDF5
        fReader = G4Hdf5AnalysisReader::Instance();
#else
        RMGLog::Out(RMGLog::fatal, "HDF5 input not available, please recompile GEANT4 with HDF5 support");
#endif
    }
    else if (ext == "csv") fReader = G4CsvAnalysisReader::Instance();
    else if (ext == "xml") fReader = G4XmlAnalysisReader::Instance();
    else {
        RMGLog::OutFormat(RMGLog::fatal, "File Extension '.{}' not recognized!");
    }

    if (RMGLog::GetLogLevelScreen() <= RMGLog::debug) fReader->SetVerboseLevel(10);

    fReader->SetFileName(name);
}

bool RMGVertexFromFile::GeneratePrimariesVertex(G4ThreeVector& vertex) {

    auto ntupleid = fReader->GetNtuple("vertices");
    if (ntupleid >= 0) {
        double xpos, ypos, zpos;
        fReader->SetNtupleDColumn("xpos", xpos);
        fReader->SetNtupleDColumn("ypos", ypos);
        fReader->SetNtupleDColumn("zpos", zpos);

        if (fReader->GetNtupleRow()) {
            vertex = G4ThreeVector{xpos, ypos, zpos};
            return true;
        }
        else RMGLog::Out(RMGLog::error, "No more vertices available in input file!");
        vertex = RMGVVertexGenerator::kDummyPrimaryPosition;
        return false;
    }
    else {
        RMGLog::Out(RMGLog::fatal, "Ntuple named 'vertices' could not be found in input file!");
        return false;
    }
}

void RMGVertexFromFile::DefineCommands() {

    fMessenger = std::make_unique<G4GenericMessenger>(this,
            "/RMG/Confinement/FromFile/", "Commands for controlling reading event vertex positions from file");

    fMessenger->DeclareMethod("FileName", &RMGVertexFromFile::OpenFile)
        .SetGuidance("Set name of the file containing vertex positions. See the documentation for a specification of the format.")
        .SetParameterName("filename", false)
        .SetStates(G4State_PreInit, G4State_Idle);
}
