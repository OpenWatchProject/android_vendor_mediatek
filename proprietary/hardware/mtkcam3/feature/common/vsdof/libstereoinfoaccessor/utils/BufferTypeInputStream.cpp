//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/BufferTypeInputStream"

#include "BufferTypeInputStream.h"
#include "StereoLog.h"
#include "BufferManager.h"

using namespace stereo;
using namespace android;

BufferTypeInputStream::BufferTypeInputStream(StereoBuffer_t &buf) {
    if (!buf.isValid()) {
        StereoLogE("cannot use enpty buffer");
        throw std::invalid_argument("cannot use empty buffer");
    }
    this->buf = buf.data;
    pos = 0;
    count = buf.size;
    StereoLogV("<ByteArrayInputStreamExt> new instance, buf count: %d", count);
}

BufferTypeInputStream::~BufferTypeInputStream() {
}

int BufferTypeInputStream::read() {
    Mutex::Autolock lock(mLock);
    StereoLogV("read, pos = %d", pos);
    return (pos < count) ? (buf[pos++] & 0xff) : -1;
}

int BufferTypeInputStream::read(StereoBuffer_t &b, int offset, int len) {
    Mutex::Autolock lock(mLock);
    StereoLogV("read, pos = %d, len = %d", pos, len);
    if (offset < 0 || len < 0) {
        return -1;
    }

    if (pos >= count) {
        return -1;
    }

    int avail = count - pos;
    if (len > avail) {
        len = avail;
    }
    if (len <= 0) {
        return 0;
    }
    BufferManager::createBuffer(len, b);
    memcpy(b.data, buf + pos, len);
    pos += len;
    return len;
}

int BufferTypeInputStream::read(StereoBuffer_t &b) {
    Mutex::Autolock lock(mLock);
    if (!b.isValid()) {
        return -1;
    }
    int len = b.size;
    StereoLogV("read, pos = %d, len = %d", pos, len);

    if (pos >= count) {
        return -1;
    }

    int avail = count - pos;
    if (len > avail) {
        len = avail;
    }
    if (len <= 0) {
        return 0;
    }
    memcpy(b.data, buf + pos, len);
    pos += len;
    return len;
}

long BufferTypeInputStream::getFilePointer() {
    Mutex::Autolock lock(mLock);
    return pos;
}

void BufferTypeInputStream::seek(long offset) {
    Mutex::Autolock lock(mLock);
    if (offset > count - 1) {
        throw std::invalid_argument(
            "offset out of buffer range, offset: %ld, buffer count: %ld");
    }
    StereoLogV("seekto %ld", offset);
    pos = (int)offset;
}

long BufferTypeInputStream::skip(long n) {
    long k = count - pos;
    if (n < k) {
        k = n < 0 ? 0 : n;
    }

    pos += k;
    StereoLogV("skip %ld bytes, pos = %d", n, pos);
    return k;
}

int BufferTypeInputStream::readUnsignedShort() {
    StereoLogV("readUnsignedShort");
    int hByte = read();
    int lByte = read();
    return hByte << 8 | lByte;
}