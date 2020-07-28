//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/BufferManager"

#include "BufferManager.h"
#include "StereoLog.h"
#include <utils/Trace.h>

using namespace stereo;
using namespace android;

#define ATRACE_TAG ATRACE_TAG_APP

// default big buffer size is 2MB
#define DEFAULT_BIG_BUFFER_SIZE 2 * 1024 * 1024

/* StereoBigBuffer */
StereoBigBuffer::StereoBigBuffer(S_UINT32 size) {
    data = new S_UINT8[size];
    totalSize = size;
    next = 0;
    availables = totalSize;
    StereoLogI("<StereoBigBuffer> new, tid: %d, addr: %p, totalSize: %d",
        gettid(), data, totalSize);
}

StereoBigBuffer::~StereoBigBuffer() {
    if (data != nullptr) {
        delete[] data;
        StereoLogI("<~StereoBigBuffer> delete StereoBigBuffer, totalSize = %d", totalSize);
    }
}

bool StereoBigBuffer::isAvaliable(S_UINT32 size) {
    return availables >= size;
}

void StereoBigBuffer::allocate(S_UINT32 size, StereoBuffer_t &buffer) {
    if (isAvaliable(size)) {
        buffer.data = data + next;
        buffer.size = size;
        next += size;
        availables = availables - size;
        StereoLogD("<allocate> allocate, tid: %d, addr: %p, size: %d", gettid(), buffer.data, size);
    }
}
/* End Of StereoBigBuffer */

android::Mutex BufferManager::mBufferLock;
StereoMap<pid_t, StereoVector<StereoBigBuffer*>*> BufferManager::mBuffers;

StereoVector<StereoBigBuffer*>* BufferManager::findBigBuffers() {
    pid_t tid = gettid();
    auto iter = mBuffers.find(tid);
    StereoVector<StereoBigBuffer*>* subBuffers;
    if (iter != mBuffers.end()) {
        subBuffers = iter->second;
    } else {
        subBuffers = new StereoVector<StereoBigBuffer*>();
        mBuffers.insert(
            StereoMap<pid_t, StereoVector<StereoBigBuffer*>*>::value_type(tid, subBuffers));
    }
    return subBuffers;
}

StereoVector<StereoBigBuffer*>* BufferManager::popBigBuffers() {
    pid_t tid = gettid();
    auto iter = mBuffers.find(tid);
    if (iter != mBuffers.end()) {
        StereoVector<StereoBigBuffer*>* subBuffers = iter->second;
        mBuffers.erase(iter);
        return subBuffers;
    }
    return nullptr;
}

void BufferManager::pushBigBuffer(StereoBigBuffer* pBuffer) {
    StereoVector<StereoBigBuffer*>* subBuffers = findBigBuffers();
    subBuffers->push_back(pBuffer);
}

void BufferManager::releaseAll() {
    ATRACE_NAME(">>>>BufferManager-releaseAll");
    Mutex::Autolock lock(mBufferLock);
    StereoVector<StereoBigBuffer*>* subBuffers = popBigBuffers();
    if (subBuffers == nullptr) {
        StereoLogW("cannot pop buffers");
        return;
    }
    StereoLogD("releaseAll, tid: %d, buffers size: %d", gettid(), subBuffers->size());
    for (auto iter = subBuffers->cbegin(); iter != subBuffers->cend(); iter++) {
        StereoBigBuffer* buffer = *iter;
        if (buffer != nullptr) {
            delete buffer;
        }
    }
    subBuffers->clear();
    delete subBuffers;
}

void BufferManager::createBuffer(S_UINT32 size, StereoBuffer_t &buffer) {
    Mutex::Autolock lock(mBufferLock);

    // find avaliable big buffer
    StereoBigBuffer* bigBuffer = nullptr;
    bool foundBigBuffer = false;

    // if wanted buffer size > DEFAULT_BIG_BUFFER_SIZE,
    // directly new StereoBigBuffer with specified size
    if (size > DEFAULT_BIG_BUFFER_SIZE) {
        bigBuffer = new StereoBigBuffer(size);
        pushBigBuffer(bigBuffer);
        foundBigBuffer = true;
    }

    // try to find available buffer in existed StereoBigBuffers
    if (!foundBigBuffer) {
        StereoVector<StereoBigBuffer*>* subBuffers = findBigBuffers();
        for (auto iter = subBuffers->cbegin(); iter != subBuffers->cend(); iter++) {
            if ((*iter)->isAvaliable(size)) {
                bigBuffer = *iter;
                foundBigBuffer = true;
                break;
            }
        }
    }

    // if can not find available buffer, new StereoBigBuffer with default buffer size
    if (!foundBigBuffer) {
        bigBuffer = new StereoBigBuffer(DEFAULT_BIG_BUFFER_SIZE);
        pushBigBuffer(bigBuffer);
        foundBigBuffer = true;
    }

    if (foundBigBuffer) {
        bigBuffer->allocate(size, buffer);
    } else {
        StereoLogE("<createBuffer> createBuffer failed");
    }
}