#ifndef __MTK_OMX_BUF_Q_H__
#define __MTK_OMX_BUF_Q_H__

#include <log/log.h>
#include <cutils/properties.h>
#include <utils/Vector.h>
#include <vector>

#include "osal_utils.h"

class MtkOmxBufQ
{
public:
    MtkOmxBufQ();
    //MtkOmxBufQ(int id);
    ~MtkOmxBufQ();

    typedef enum
    {
        MTK_OMX_VENC_BUFQ_INPUT = 0,
        MTK_OMX_VENC_BUFQ_OUTPUT,
        MTK_OMX_VENC_BUFQ_CONVERT_OUTPUT,
        MTK_OMX_VENC_BUFQ_VENC_INPUT,
    } MtkOmxBufQId;
#if CPP_STL_SUPPORT
    vector<int>         mBufQ;
#endif//CPP_STL_SUPPORT
#if ANDROID
    Vector<int>         mBufQ;
#endif//ANDROID
    MtkOmxBufQId        mId;
    int                 mPendingNum;
    pthread_mutex_t     mBufQLock;

    void    QueueBufferFront(int index);//for input buffer
    void    QueueBufferBack(int index);//for output buffer
    int     DequeueBuffer();
    int     HandleThisBuffer(int index);//push index to bufQ
    int     HandleThisBufferDone();//decrease pending number
    void    DumpBufQ(bool debug, const char* title);
    bool    IsEmpty();
    void    Push(int index);
    void    PushFront(int index);
    int     operator[](size_t index);
    size_t  Size();
    void    Clear();
    int     PendingNum(int d=0);
    void    Lock(); // mBufQBigLock
    void    Unlock(); // mBufQBigLock

private:
    MtkOmxBufQ(const MtkOmxBufQ &);
    MtkOmxBufQ &operator=(const MtkOmxBufQ &);

    pthread_mutex_t     mBufQBigLock; // workaround
};

#endif