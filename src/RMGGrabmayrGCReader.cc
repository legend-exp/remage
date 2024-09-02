#include "RMGGrabmayrGCReader.hh"

#include "G4Tokenizer.hh"
#include "Randomize.hh"

#include "RMGLog.hh"


RMGGrabmayrGCReader* RMGGrabmayrGCReader::instance = nullptr;

RMGGrabmayrGCReader* RMGGrabmayrGCReader::GetInstance() {
  if (instance == nullptr) { instance = new RMGGrabmayrGCReader(); }
  return instance;
}

RMGGrabmayrGCReader::RMGGrabmayrGCReader() { DefineCommands(); }

RMGGrabmayrGCReader::~RMGGrabmayrGCReader() { CloseFiles(); }

void RMGGrabmayrGCReader::CloseFiles() {
  RMGLog::Out(RMGLog::detail, "Closing gamma cascade files");
  for (const auto& el : fCascadeFiles) {
    if (el.second->is_open()) { el.second->close(); }
  }
}

// Returns true if there exists a cascade file for the Isotope Z, A
G4bool RMGGrabmayrGCReader::IsApplicable(G4int z, G4int a) {
  std::pair<G4int, G4int> key = std::make_pair(z, a);
  auto it = fCascadeFiles.find(key);
  return it != fCascadeFiles.end();
}

// Returns the next cascade for the Isotope Z, A.
GammaCascadeLine RMGGrabmayrGCReader::GetNextEntry(G4int z, G4int a) {
  // Find the corresponding file
  std::pair<G4int, G4int> key = std::make_pair(z, a);
  auto it = fCascadeFiles.find(key);
  if (it == fCascadeFiles.end())
    RMGLog::Out(RMGLog::fatal, "No Cascade exists for Isotope Z: " + std::to_string(z) +
                                   " A: " + std::to_string(a) + " Exit!");
  // read next line from file
  std::string line;
  do {
    if (!std::getline(*(it->second), line)) {
      // if end-of-file is reached, reset file and read first line
      RMGLog::Out(RMGLog::detail, "Gamma cascade file EOF reached, re-opening the file");
      it->second->clear();                 // clear EOF flag
      it->second->seekg(0, std::ios::beg); // move to beginning of file
      if (!std::getline(*(it->second), line)) {
        RMGLog::Out(RMGLog::fatal, "Failed to read next line after re-opening the file. Exit!");
      }
    }
  } while (line[0] == '%' || (line.find("version") !=
                                 std::string::npos)); // This could be outsourced to SetStartLocation

  // parse line and return as struct
  GammaCascadeLine gamma_cascade;
  std::istringstream iss(line);
  iss >> gamma_cascade.en >> gamma_cascade.ex >> gamma_cascade.m >> gamma_cascade.em;
  gamma_cascade.eg.reserve(gamma_cascade.m);
  int eg_value;
  for (int i = 0; i < gamma_cascade.m; i++) {
    if (!(iss >> eg_value)) {
      RMGLog::Out(RMGLog::fatal, "Failed to read gamma energy from file. Exit!");
    }
    gamma_cascade.eg.push_back(eg_value);
  }
  return gamma_cascade;
}


// Is there a better way dealing with the unique pointer?
std::unique_ptr<std::ifstream> RMGGrabmayrGCReader::SetStartLocation(
    std::unique_ptr<std::ifstream> file) {
  if (!file || !file->is_open())
    RMGLog::Out(RMGLog::fatal, "The file is not open to set start location! Exit.");
  file->clear();                 // clear EOF flag
  file->seekg(0, std::ios::beg); // move to beginning of file
  // Skip Header
  std::string line;
  int header_length = 0;
  do {
    std::getline(*file, line);
    header_length++;
  } while (line[0] == '%' || (line.find("version") != std::string::npos));

  // In case the Random start location macro is set
  if (fGammaCascadeRandomStartLocation) {
    int n_entries_in_file = 0;
    // Seems excessiv to read through the entire file, there has to be a quicker way?
    while (std::getline(*file, line)) n_entries_in_file++;

    file->clear();                 // clear EOF flag
    file->seekg(0, std::ios::beg); // move to beginning of file

    int start_location = (int)(n_entries_in_file * G4UniformRand() + header_length);

    RMGLog::Out(RMGLog::detail, "Random start location: ", start_location);
    for (int j = 0; j < start_location; j++) std::getline(*file, line);
  }
  return std::move(file);
}

void RMGGrabmayrGCReader::SetGammaCascadeFile(const G4String& params) {
  // Convert token to Z, A and path/to/file
  G4Tokenizer tokenizer(params);
  G4String zStr = tokenizer(",");
  G4String aStr = tokenizer(",");
  G4String file_name = tokenizer(" \t\n");
  G4int z = G4UIcommand::ConvertToInt(zStr);
  G4int a = G4UIcommand::ConvertToInt(aStr);

  RMGLog::Out(RMGLog::detail, "Opening file ", file_name);
  std::unique_ptr<std::ifstream> file = std::make_unique<std::ifstream>(file_name);

  if (z == 0 || a == 0)
    RMGLog::Out(RMGLog::fatal, "Isotope Z:" + std::to_string(z) + " A: " + std::to_string(a) +
                                   " does not exist. Are you sure you used the correct format?");
  if (!file || !file->is_open())
    RMGLog::Out(RMGLog::fatal, "Gamma cascade file: " + file_name + " not found! Exit.");

  file = SetStartLocation(std::move(file));

  fCascadeFiles.insert({{z, a}, std::move(file)});
}

void RMGGrabmayrGCReader::RandomizeFiles() {
  RMGLog::Out(RMGLog::detail, "(Un)-Randomizing start locations");
  for (auto& el : fCascadeFiles) { el.second = SetStartLocation(std::move(el.second)); }
}

void RMGGrabmayrGCReader::SetGammaCascadeRandomStartLocation(const int answer) {
  fGammaCascadeRandomStartLocation = answer;
  RMGLog::Out(RMGLog::detail,
      "setting fGammaCascadeRandomStartLocation to: ", fGammaCascadeRandomStartLocation);
  RandomizeFiles();
}

void RMGGrabmayrGCReader::DefineCommands() {
  fMessenger = std::make_unique<G4GenericMessenger>(this, "/RMG/GrabmayrGammaCascades/",
      "Control Peters gamma cascade model");

  fMessenger->DeclareMethod("SetGammaCascadeFile", &RMGGrabmayrGCReader::SetGammaCascadeFile)
      .SetGuidance("Set the Z, A and /path/to/file for the gamma cascade upon neutron capture on "
                   "Isotope Z, A Format: Z,A,/path/to/file")
      .SetParameterName("Z,A,/path/to/file", false)
      .SetDefaultValue("64,155,/path/to/file.txt")
      .SetStates(G4State_PreInit, G4State_Idle);


  fMessenger
      ->DeclareMethod("SetGammaCascadeRandomStartLocation",
          &RMGGrabmayrGCReader::SetGammaCascadeRandomStartLocation)
      .SetGuidance("Set the whether the start location in the gamma cascade file is random or not")
      .SetGuidance("0 = don't")
      .SetGuidance("1 = do")
      .SetCandidates("0 1")
      .SetDefaultValue("0")
      .SetStates(G4State_PreInit, G4State_Idle);
}
