#include "RMGReaderMUSUN.hh"
#include "RMGLog.hh"

RMGReaderMUSUN* RMGReaderMUSUN::kInstance = NULL;

void RMGReaderMUSUN::SetInputFile(std::string fileName){
    kFileName = fileName;
    if(kInStream.is_open())
        kInStream.close();
    kInStream.open(kFileName.c_str());
}

MUSUN_OUTPUT RMGReaderMUSUN::ReadNextLine(){
    MUSUN_OUTPUT output;

    if(kInStream.peek() == EOF)
        RMGLog::Out(RMGLog::fatal, "Trying to read from the MUSUN file even though it is EOF! Test for isEOF before.");

    kInStream >> output.nEvent >> output.particleID >> output.energy >> output.x >> output.y >> output.z >> output.theta >> output.phi;
    return output;
}