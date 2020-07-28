#ifndef BUFFER_TYPE_OUTPUT_STREAM_H
#define BUFFER_TYPE_OUTPUT_STREAM_H

#include <utils/Mutex.h>
#include "types.h"
#include "BufferManager.h"

namespace stereo {

class BufferTypeOutputStream {

public:
    BufferTypeOutputStream();
    explicit BufferTypeOutputStream(int size);
    virtual ~BufferTypeOutputStream();
    void write(int b);
    void write(const StereoBuffer_t &b);
    void writeShort(int val);
    void reset();
    int size();
    void toBuffer(StereoBuffer_t &out);

private:
    mutable android::Mutex mLock;
    StereoVector<S_UINT8> buf;
    int count = 0;

    void ensureCapacity(int minCapacity);
    void grow(int minCapacity);
};

}

#endif