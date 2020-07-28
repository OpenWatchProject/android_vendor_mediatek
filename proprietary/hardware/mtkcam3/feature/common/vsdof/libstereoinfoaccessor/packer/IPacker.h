#ifndef IPACKER_H
#define IPACKER_H

// include structure file
#include <mtkcam3/feature/stereo/libstereoinfoaccessor/IncludeStruct.h>

namespace stereo {

class IPacker {

public:
    virtual ~IPacker() {}
    virtual void pack() = 0;
    virtual void unpack() = 0;
};

}

#endif

