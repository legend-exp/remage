#ifndef _RMG_GENERATOR_MUSUN_READER_HH_
#define _RMG_GENERATOR_MUSUN_READER_HH_

#include <iostream>
#include <fstream>

struct MUSUN_OUTPUT{
    int nEvent;
    int particleID;
    float energy;
    float x;
    float y;
    float z;
    float theta;
    float phi;
};

class RMGReaderMUSUN {

    private:

        RMGReaderMUSUN(){};

        std::string kFileName;
        std::ifstream kInStream;
        static RMGReaderMUSUN* kInstance;

    public:

        RMGReaderMUSUN(const RMGReaderMUSUN &) = delete;
        RMGReaderMUSUN &operator=(const RMGReaderMUSUN &) = delete;

        static RMGReaderMUSUN *getInstance()
        {
            if(kInstance == NULL)
                kInstance = new RMGReaderMUSUN();
            return kInstance;
        }
        void SetInputFile(std::string fileName);
        MUSUN_OUTPUT ReadNextLine();
        bool IsEOF(){return kInStream.peek() == EOF;}

};


#endif