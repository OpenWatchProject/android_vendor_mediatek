#ifndef CUSTOMIZED_META_H
#define CUSTOMIZED_META_H

#include "types.h"
#include "BufferManager.h"

namespace stereo {

class CustomizedMeta {

public:

    explicit CustomizedMeta(BufferMapPtr custData);

    virtual ~CustomizedMeta();

    bool getPropertyBuffer(const StereoString &name, StereoBuffer_t &buffer);

    void setPropertyBuffer(const StereoString &name, StereoBuffer_t &buffer);

    BufferMapPtr serialize();

private:
    BufferMapPtr pCustData;
};

}

#endif