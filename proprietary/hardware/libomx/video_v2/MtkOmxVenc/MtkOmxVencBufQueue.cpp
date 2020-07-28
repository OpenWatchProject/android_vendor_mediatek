
#include "MtkOmxVencBufQueue.h"

#define LOG_TAG "MtkOmxVencBufQ"
#define BUFQ_LOGD(B, ...) if(B) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

MtkOmxBufQ::MtkOmxBufQ()
    : mId(MTK_OMX_VENC_BUFQ_INPUT),
      mPendingNum(0)
{
    INIT_MUTEX(mBufQLock);
    INIT_MUTEX(mBufQBigLock);
    mBufQ.clear();
}

MtkOmxBufQ::~MtkOmxBufQ()
{
    DESTROY_MUTEX(mBufQLock);
    DESTROY_MUTEX(mBufQBigLock);
    mBufQ.clear();
}

int MtkOmxBufQ::DequeueBuffer()
{
    int output_idx = -1;
    LOCK(mBufQLock);

    if (mBufQ.size() <= 0)
    {
        UNLOCK(mBufQLock);
        return output_idx;
    }
#if CPP_STL_SUPPORT
    output_idx = *(mBufQ.begin());
    mBufQ.erase(mBufQ.begin());
#endif

#if ANDROID
    output_idx = mBufQ[0];
    mBufQ.removeAt(0);
#endif

    UNLOCK(mBufQLock);
    return output_idx;
}

void MtkOmxBufQ::QueueBufferBack(int index)
{
    LOCK(mBufQLock);
#if CPP_STL_SUPPORT
    mBufQ.push_back(index);
#endif

#if ANDROID
    mBufQ.push(index);
#endif

    UNLOCK(mBufQLock);
}

void MtkOmxBufQ::QueueBufferFront(int index)
{
    LOCK(mBufQLock);
#if CPP_STL_SUPPORT
    mBufQ.push_front(index);
#endif

#if ANDROID
    mBufQ.insertAt(index, 0);
#endif

    UNLOCK(mBufQLock);
}

bool MtkOmxBufQ::IsEmpty()
{
    bool ret = false;
    LOCK(mBufQLock);
#if CPP_STL_SUPPORT
    ret = (bool)mBufQ.empty();
#endif
#if ANDROID
    ret = (bool)mBufQ.isEmpty();
#endif
    UNLOCK(mBufQLock);
    return ret;
}

void MtkOmxBufQ::Push(int index)
{
    LOCK(mBufQLock);
#if CPP_STL_SUPPORT
    mBufQ.push_back(index);
#endif
#if ANDROID
    mBufQ.push(index);
#endif

    UNLOCK(mBufQLock);
    return;
}

void MtkOmxBufQ::PushFront(int index)
{
    LOCK(mBufQLock);
#if CPP_STL_SUPPORT
    mBufQ.push_front(index);
#endif
#if ANDROID
    mBufQ.insertAt(index, 0);
#endif

    UNLOCK(mBufQLock);
    return;
}

int MtkOmxBufQ::operator[](size_t index)
{
    int ret = -1;
    LOCK(mBufQLock);
    if(index >= 0 && index < mBufQ.size())
        ret = mBufQ[index];
    UNLOCK(mBufQLock);
    return ret;
}

void MtkOmxBufQ::DumpBufQ(bool debug, const char* title)
{
    if(!debug) return;

    char bytesused_buf[64] = {0};
    LOCK(mBufQLock);
    for(int i=0; i<mBufQ.size(); i++)
    {
        char tmp[32] = {0};
        int index = mBufQ[i];
        snprintf(tmp, sizeof(tmp), ",%d", index);
        if(strlen(bytesused_buf) + strlen(tmp) < sizeof(bytesused_buf))
        {
             strncat(bytesused_buf, tmp, strlen(tmp));
        }
    }
    UNLOCK(mBufQLock);
    BUFQ_LOGD(debug, "%s: %s", title, bytesused_buf+1);
}

size_t MtkOmxBufQ::Size()
{
    LOCK(mBufQLock);
    size_t ret = mBufQ.size();
    UNLOCK(mBufQLock);
    return ret;
}

void MtkOmxBufQ::Clear()
{
    LOCK(mBufQLock);
    mBufQ.clear();
    UNLOCK(mBufQLock);
    return;
}

int MtkOmxBufQ::PendingNum(int d)
{
    int ret = 0;
    LOCK(mBufQLock);
    if(d != 0) mPendingNum += d;
    ret = mPendingNum;
    UNLOCK(mBufQLock);
    return ret;
}

void MtkOmxBufQ::Lock() {LOCK(mBufQBigLock);}
void MtkOmxBufQ::Unlock() {UNLOCK(mBufQBigLock);}
