#ifndef BUFFER_TYPE_INPUT_STREAM_H
#define BUFFER_TYPE_INPUT_STREAM_H


#include <utils/Mutex.h>
#include "BufferManager.h"

namespace stereo {

class BufferTypeInputStream {

public:
    explicit BufferTypeInputStream(StereoBuffer_t &buf);
    virtual ~BufferTypeInputStream();
    int read();
    int read(StereoBuffer_t &b, int offset, int len);
    int read(StereoBuffer_t &b);
    long getFilePointer();
    void seek(long offset);
    long skip(long n);
    int readUnsignedShort();

private:
    mutable android::Mutex mLock;
    S_UINT8 *buf;
    int count;
    int pos;
};

}

#endif