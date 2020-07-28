//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/BufferTypeOutputStream"

#include "BufferTypeOutputStream.h"
#include "StereoLog.h"
#include "BufferManager.h"

using namespace stereo;
using namespace android;

BufferTypeOutputStream::BufferTypeOutputStream() {
    BufferTypeOutputStream(32);
}

BufferTypeOutputStream::BufferTypeOutputStream(int size) {
    if (size < 0) {
        StereoLogW("<BufferTypeOutputStream> size can not less than 0");
        return;
    }
    buf.resize(size);
    StereoLogV("<BufferTypeOutputStream> buf size: %d", buf.size());
}

BufferTypeOutputStream::~BufferTypeOutputStream() {
}

void BufferTypeOutputStream::ensureCapacity(int minCapacity) {
    // overflow-conscious code
    int bufSize = buf.size();
    if ((minCapacity - bufSize) > 0) {
        grow(minCapacity);
    }
}

void BufferTypeOutputStream::grow(int minCapacity) {
    // overflow-conscious code
    int oldCapacity = buf.size();
    int newCapacity = oldCapacity << 1;
    if (newCapacity - minCapacity < 0) {
        newCapacity = minCapacity;
    }
    StereoLogV("<grow> new size: %d", newCapacity);
    buf.resize(newCapacity);
}

void BufferTypeOutputStream::write(int b) {
    Mutex::Autolock lock(mLock);
    ensureCapacity(count + 1);
    StereoLogV("<write> buf size: %d, count: %d, b: %d", buf.size(), count, b);
    buf[count] = (char)b;
    count += 1;
}

void BufferTypeOutputStream::write(const StereoBuffer_t &b) {
    Mutex::Autolock lock(mLock);
    if (!b.isValid()) {
        return;
    }
    int len = b.size;
    StereoLogV("<write> buf count: %d, len: %d", count, len);
    ensureCapacity(count + len);
    memcpy(&buf[count], b.data, len);
    count += len;
}

void BufferTypeOutputStream::writeShort(int val) {
    StereoLogV("<writeShort> val: %d", val);
    int hByte = val >> 8;
    int lByte = val & 0xff;
    write(hByte);
    write(lByte);
}

void BufferTypeOutputStream::reset() {
    Mutex::Autolock lock(mLock);
    count = 0;
}

int BufferTypeOutputStream::size() {
    Mutex::Autolock lock(mLock);
    return count;
}

void BufferTypeOutputStream::toBuffer(StereoBuffer_t &out) {
    StereoLogV("<toBuffer>");
    Mutex::Autolock lock(mLock);
    BufferManager::createBuffer(count, out);
    memcpy(out.data, &buf[0], count);
}