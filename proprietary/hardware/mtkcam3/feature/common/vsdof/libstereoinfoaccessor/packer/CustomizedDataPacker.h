#ifndef CUSTOMIZED_DATA_PACKER_H
#define CUSTOMIZED_DATA_PACKER_H

#include "IPacker.h"

namespace stereo {

/**
 * Customized data packer.
 */
class CustomizedDataPacker : public IPacker {

public:
    /**
     * CustomizedDataPacker constructor.
     * @param packInfo
     *            PackInfo
     */
    explicit CustomizedDataPacker(PackInfo *packInfo);
    virtual ~CustomizedDataPacker();
    virtual void pack();
    virtual void unpack();

private:
    PackInfo *pPackInfo;
    StereoVector<StereoBuffer_t>* pack(StereoBuffer_t &bufferData, StereoString type);
};

}

#endif