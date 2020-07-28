#ifndef PACKER_MANAGER_H
#define PACKER_MANAGER_H

#include "IPacker.h"

namespace stereo {

class PackerManager {
public:

    /**
     * Pack work flow.
     * @param packInfo
     *            PackInfo
     * @return JPG buffer
     */
    StereoBuffer_t pack(PackInfo *packInfo);

    /**
     * Unpack work flow.
     * @param src
     *            JPG buffer
     * @return PackInfo
     */
    PackInfo* unpack(StereoBuffer_t &src);
};

}
#endif