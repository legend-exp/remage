// Copyright (C) 2024 Moritz Neuberger <https://orcid.org/0009-0001-8471-9076>
// Copyright (C) 2024 Eric Esch <https://orcid.org/0009-0000-4920-9313>
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option) any
// later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


#include "RMGGrabmayrGCReader.hh"

#include <algorithm>
#include <cstdlib>
#include <sstream>

#include "G4Tokenizer.hh"
#include "Randomize.hh"

#include "RMGLog.hh"


G4ThreadLocal RMGGrabmayrGCReader* RMGGrabmayrGCReader::instance = nullptr;

RMGGrabmayrGCReader* RMGGrabmayrGCReader::GetInstance() {
  if (instance == nullptr) { instance = new RMGGrabmayrGCReader(); }
  return instance;
}

RMGGrabmayrGCReader::RMGGrabmayrGCReader() { DefineCommands(); }

RMGGrabmayrGCReader::~RMGGrabmayrGCReader() { CloseFiles(); }

void RMGGrabmayrGCReader::CloseFiles() {
  RMGLog::Out(RMGLog::detail, "Closing gamma cascade files");
  for (const auto& el : fCascadeFiles) {
    for (const auto& entry : el.second) {
      if (entry.file && entry.file->is_open()) { entry.file->close(); }
    }
  }
}

// Returns true if there exists a cascade file for the Isotope Z, A
G4bool RMGGrabmayrGCReader::IsApplicable(G4int z, G4int a) {
  std::pair<G4int, G4int> key = std::make_pair(z, a);
  auto it = fCascadeFiles.find(key);
  return it != fCascadeFiles.end();
}

// Returns the next cascade for the Isotope Z, A, drawing from the cascade file whose neutron
// kinetic-energy range contains neutron_energy_keV.
GammaCascadeLine RMGGrabmayrGCReader::GetNextEntry(G4int z, G4int a, G4double neutron_energy_keV) {
  // Find the corresponding isotope
  std::pair<G4int, G4int> key = std::make_pair(z, a);
  auto it = fCascadeFiles.find(key);
  if (it == fCascadeFiles.end())
    RMGLog::OutFormat(RMGLog::fatal, "Isotope Z: {} A: {} does not exist.", z, a);
  auto& entries = it->second; // vector<GammaCascadeFileEntry>, non-empty, sorted ascending

  // Select the cascade file for this neutron energy.
  GammaCascadeFileEntry* selected = nullptr;
  if (entries.size() == 1) {
    // Legacy single-file case: use it for all neutron energies (energy range ignored).
    selected = &entries.front();
  } else {
    for (auto& entry : entries) {
      if (neutron_energy_keV >= entry.en_low && neutron_energy_keV < entry.en_high) {
        selected = &entry;
        break;
      }
    }
    // Out of range: clamp to the nearest (first/last) bin and warn once per isotope.
    if (selected == nullptr) {
      selected = (neutron_energy_keV < entries.front().en_low) ? &entries.front() : &entries.back();
      if (fOutOfRangeWarned.insert(key).second) {
        RMGLog::OutFormat(
            RMGLog::warning,
            "Neutron kinetic energy {} keV is outside the registered cascade ranges for isotope "
            "Z: {} A: {}; clamping to the [{}, {}) keV file.",
            neutron_energy_keV,
            z,
            a,
            selected->en_low,
            selected->en_high
        );
      }
    }
  }
  std::ifstream& stream = *selected->file;

  // read next line from file
  std::string line;
  do { // NOLINT(cppcoreguidelines-avoid-do-while)
    if (!std::getline(stream, line)) {
      // if end-of-file is reached, reset file and read first line
      RMGLog::Out(RMGLog::debug_event, "Gamma cascade file EOF reached, re-opening the file");
      stream.clear();                 // clear EOF flag
      stream.seekg(0, std::ios::beg); // move to beginning of file
      if (!std::getline(stream, line)) {
        RMGLog::Out(RMGLog::fatal, "Failed to read next line after re-opening the file. Exit!");
      }
    }
  } while (
      line.empty() || line[0] == '%' ||
      (line.find("version") != std::string::npos)); // This could be outsourced to SetStartLocation

  // parse line and return as struct. All fields are integers.
  GammaCascadeLine gamma_cascade{};
  const char* pos = line.c_str();
  auto next_int = [&pos](G4int& out) {
    char* endptr = nullptr;
    const long val = std::strtol(pos, &endptr, 10);
    if (endptr == pos) return false; // no digits consumed
    pos = endptr;
    out = static_cast<G4int>(val);
    return true;
  };

  if (!next_int(gamma_cascade.en) || !next_int(gamma_cascade.ex) || !next_int(gamma_cascade.m) ||
      !next_int(gamma_cascade.em)) {
    RMGLog::Out(RMGLog::fatal, "Failed to parse gamma cascade header fields. Exit!");
  }
  gamma_cascade.eg.reserve(gamma_cascade.m);
  for (int i = 0; i < gamma_cascade.m; i++) {
    G4int eg_value = 0;
    if (!next_int(eg_value)) {
      RMGLog::Out(RMGLog::fatal, "Failed to read gamma energy from file. Exit!");
    }
    gamma_cascade.eg.push_back(eg_value);
  }
  return gamma_cascade;
}


void RMGGrabmayrGCReader::SetStartLocation(std::ifstream& file) const {
  if (!file.is_open())
    RMGLog::Out(RMGLog::fatal, "The file is not open to set start location! Exit.");
  file.clear();                 // clear EOF flag
  file.seekg(0, std::ios::beg); // move to beginning of file
  // Skip Header
  std::string line;
  do { // NOLINT(cppcoreguidelines-avoid-do-while)
    std::getline(file, line);
  } while (line[0] == '%' || (line.find("version") != std::string::npos));

  // In case the Random start location macro is set
  if (fGammaCascadeRandomStartLocation) {
    // Single pass over the remaining entries, recording each line's stream offset, so we can
    // seek straight to a random entry instead of rewinding and re-reading from the top.
    std::vector<std::streampos> entry_offsets;
    std::streampos offset = file.tellg();
    while (std::getline(file, line)) {
      entry_offsets.push_back(offset);
      offset = file.tellg();
    }
    file.clear(); // clear EOF flag

    if (!entry_offsets.empty()) {
      const auto start_location = (std::size_t)(entry_offsets.size() * G4UniformRand());
      RMGLog::Out(RMGLog::detail, "Random start location: ", start_location);
      file.seekg(entry_offsets[start_location]);
    }
  }
}

void RMGGrabmayrGCReader::RegisterCascadeFile(
    const G4int z,
    const G4int a,
    const G4String& file_name,
    const G4double en_low,
    const G4double en_high
) {
  RMGLog::Out(RMGLog::detail, "Opening file ", file_name);
  std::unique_ptr<std::ifstream> file = std::make_unique<std::ifstream>(file_name);

  if (z == 0 || a == 0)
    RMGLog::OutFormat(RMGLog::fatal, "Isotope Z: {} A: {} does not exist.", z, a);
  if (!file || !file->is_open())
    RMGLog::Out(RMGLog::fatal, "Gamma cascade file: " + file_name + " not found! Exit.");

  SetStartLocation(*file);

  fCascadeFiles[std::make_pair(z, a)].push_back(
      GammaCascadeFileEntry{en_low, en_high, std::move(file)}
  );
}

void RMGGrabmayrGCReader::SetGammaCascadeFile(const G4int z, const G4int a, const G4String file_name) {
  // Legacy single file case
  const auto key = std::make_pair(z, a);
  fCascadeFiles.erase(key);
  fOutOfRangeWarned.erase(key);

  RegisterCascadeFile(z, a, file_name, 0.0, 0.0);
}

void RMGGrabmayrGCReader::SetGammaCascadeFilelist(
    const G4int z,
    const G4int a,
    const G4String filelist_name
) {
  const auto key = std::make_pair(z, a);
  fCascadeFiles.erase(key);
  fOutOfRangeWarned.erase(key);

  // The filelist maps a neutron kinetic-energy range to the cascade file to use for that range.
  // Each row is "<cascade filename> <E_low> <E_high>" (keV, interval [E_low, E_high)); cascade
  // filenames are resolved relative to the directory containing the filelist.
  const std::filesystem::path filelist_path(filelist_name.c_str());
  const std::filesystem::path base_dir = filelist_path.parent_path();

  std::ifstream filelist(filelist_path);
  if (!filelist.is_open())
    RMGLog::Out(RMGLog::fatal, "Gamma cascade filelist: " + filelist_name + " not found! Exit.");

  std::size_t n_entries = 0;
  std::string line;
  while (std::getline(filelist, line)) {
    if (line.empty() || line[0] == '%' || line[0] == '#') continue; // blank / comment

    std::istringstream iss(line);
    std::string file_name;
    G4double en_low = 0.0;
    G4double en_high = 0.0;
    if (!(iss >> file_name >> en_low >> en_high)) {
      RMGLog::Out(
          RMGLog::fatal,
          "Malformed gamma cascade filelist line: '" + line +
              "'. Expected: <file> <E_low> <E_high>. Exit."
      );
    }

    if (en_high <= en_low)
      RMGLog::OutFormat(
          RMGLog::fatal,
          "Invalid gamma cascade filelist range: E_low={} keV, E_high={} keV (require E_high > "
          "E_low). Exit.",
          en_low,
          en_high
      );

    const std::filesystem::path cascade_path = base_dir / file_name;
    RegisterCascadeFile(z, a, cascade_path.string(), en_low, en_high);
    n_entries++;
  }

  if (n_entries == 0)
    RMGLog::OutFormat(
        RMGLog::fatal,
        "Gamma cascade filelist {} contained no valid entries. Exit.",
        filelist_name
    );

  // Keep the per-isotope entries sorted ascending by lower energy bound
  auto& entries = fCascadeFiles[std::make_pair(z, a)];
  std::sort(
      entries.begin(),
      entries.end(),
      [](const GammaCascadeFileEntry& l, const GammaCascadeFileEntry& r) {
        return l.en_low < r.en_low;
      }
  );
}

void RMGGrabmayrGCReader::RandomizeFiles() {
  RMGLog::Out(RMGLog::detail, "(Un)-Randomizing start locations");
  for (auto& el : fCascadeFiles) {
    for (auto& entry : el.second) { SetStartLocation(*entry.file); }
  }
}

void RMGGrabmayrGCReader::SetGammaCascadeRandomStartLocation(const int answer) {
  fGammaCascadeRandomStartLocation = answer;
  RMGLog::Out(
      RMGLog::detail,
      "setting fGammaCascadeRandomStartLocation to: ",
      fGammaCascadeRandomStartLocation
  );
  RandomizeFiles();
}

void RMGGrabmayrGCReader::DefineCommands() {
  fGenericMessenger = std::make_unique<G4GenericMessenger>(
      this,
      "/RMG/GrabmayrGammaCascades/",
      "Control Peters gamma cascade model"
  );

  fGenericMessenger
      ->DeclareMethod(
          "SetGammaCascadeRandomStartLocation",
          &RMGGrabmayrGCReader::SetGammaCascadeRandomStartLocation
      )
      .SetGuidance("Set the whether the start location in the gamma cascade file is random or not")
      .SetGuidance("0 = don't")
      .SetGuidance("1 = do")
      .SetCandidates("0 1")
      .SetDefaultValue("0")
      .SetStates(G4State_PreInit, G4State_Idle)
      .SetToBeBroadcasted(true);

  // SetGammaCascadeFile cannot be defined with the G4GenericMessenger (it has too many parameters).
  fUIMessenger = std::make_unique<GCMessenger>(this);
}


RMGGrabmayrGCReader::GCMessenger::GCMessenger(RMGGrabmayrGCReader* reader) : fReader(reader) {
  fGammaFileCmd = new G4UIcommand("/RMG/GrabmayrGammaCascades/SetGammaCascadeFile", this);
  fGammaFileCmd->SetGuidance(
      "Set a gamma cascade file for neutron capture on a specified isotope. It is applied "
      "independent of the kinetic energy of the incoming neutron. Resets any already registered "
      "gamma cascades for the specific isotope."
  );

  auto p_Z = new G4UIparameter("Z", 'i', false);
  p_Z->SetGuidance("Z of isotope");
  fGammaFileCmd->SetParameter(p_Z);

  auto p_A = new G4UIparameter("A", 'i', false);
  p_A->SetGuidance("A of isotope");
  fGammaFileCmd->SetParameter(p_A);

  auto p_file = new G4UIparameter("file", 's', false);
  p_file->SetGuidance("/path/to/file of gamma cascade");
  fGammaFileCmd->SetParameter(p_file);

  fGammaFileCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fGammaFilelistCmd = new G4UIcommand("/RMG/GrabmayrGammaCascades/SetGammaCascadeFilelist", this);
  fGammaFilelistCmd->SetGuidance(
      "Set a file that lists a set of gamma cascade files for neutron capture on a specified "
      "isotope depending on the kinetic energy of the incoming neutron. remage selects the file "
      "matching the incoming neutron energy at runtime. Resets any already registered gamma "
      "cascade for the specific isotope."
  );

  auto q_Z = new G4UIparameter("Z", 'i', false);
  q_Z->SetGuidance("Z of isotope");
  fGammaFilelistCmd->SetParameter(q_Z);

  auto q_A = new G4UIparameter("A", 'i', false);
  q_A->SetGuidance("A of isotope");
  fGammaFilelistCmd->SetParameter(q_A);

  auto q_filelist = new G4UIparameter("filelist", 's', false);
  q_filelist->SetGuidance(
      "/path/to/<isotope>_ncapture_filelist.txt (cascade paths are relative to it)"
  );
  fGammaFilelistCmd->SetParameter(q_filelist);

  fGammaFilelistCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

RMGGrabmayrGCReader::GCMessenger::~GCMessenger() {
  delete fGammaFileCmd;
  delete fGammaFilelistCmd;
}

void RMGGrabmayrGCReader::GCMessenger::SetNewValue(G4UIcommand* command, G4String newValues) {
  if (command == fGammaFileCmd) GammaFileCmd(newValues);
  else if (command == fGammaFilelistCmd) GammaFilelistCmd(newValues);
}

void RMGGrabmayrGCReader::GCMessenger::GammaFileCmd(const std::string& parameters) {
  G4Tokenizer next(parameters);

  auto Z = std::stoi(next());
  auto A = std::stoi(next());
  auto file = next();

  fReader->SetGammaCascadeFile(Z, A, file);
}

void RMGGrabmayrGCReader::GCMessenger::GammaFilelistCmd(const std::string& parameters) {
  G4Tokenizer next(parameters);

  auto Z = std::stoi(next());
  auto A = std::stoi(next());
  auto filelist = next();

  fReader->SetGammaCascadeFilelist(Z, A, filelist);
}
