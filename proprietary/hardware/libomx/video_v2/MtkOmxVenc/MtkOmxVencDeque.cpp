#include "MtkOmx2V4L2.h"
#include <assert.h>
#include <errno.h>
#include <semaphore.h>

struct DequeTask;

void *MtkOmxVencDequeThread(void *pData)
{
    MtkOmxVenc *pVenc = (MtkOmxVenc *)pData;
    pVenc->mVencDqThreadTid = gettid();

    ALOGD("[0x%08x] ""MtkOmxVencDequeThread created pVenc=0x%08X, tid=%d", pVenc, (unsigned int)pVenc, gettid());
    prctl(PR_SET_NAME, (unsigned long)"MtkOmxVencDequeThread", 0, 0, 0);

    int breakhint = 0;

    while(1)
    {
        pVenc->waitIfNoEnqueued();

        int stop = pVenc->shouldThreadStop();
        if(stop)
        {
            breakhint = stop;
            break;
        }

        if (pVenc->mEncodeStarted == OMX_FALSE || pVenc->mEncoderInitCompleteFlag == OMX_FALSE)
        {
            ALOGD("[0x%08x] ""Wait for encode start.....", pVenc);
            SLEEP_MS(2);
            continue;
        }

        pVenc->doDeque();
    }
    ALOGD("[0x%08x] ""MtkOmxVencDequeThread terminated pVenc=0x%08X hint %d", pVenc, (unsigned int)pVenc, breakhint);
    return NULL;
}

int MtkOmxVenc::shouldThreadStop()
{
    if (isEosState())
    {
        return 4; // get eos buffer, encode should end
    }

    return 0;
}

void MtkOmxVenc::MtkOmxVencDeque()
{
    INIT_COND(mFlushDoneCond);
    INIT_MUTEX(mFlushDoneCondLock);

    INIT_MUTEX(mDqFrmLock);
    INIT_COND(mDqFrmCond);
    //INIT_MUTEX(mDqBsLock);
    INIT_MUTEX(mDqCvtLock);
}

void MtkOmxVenc::deMtkOmxVencDeque()
{
    DESTROY_COND(mFlushDoneCond);
    DESTROY_MUTEX(mFlushDoneCondLock);

    DESTROY_MUTEX(mDqFrmLock);
    DESTROY_COND(mDqFrmCond);
    //DESTROY_MUTEX(mDqBsLock);
    DESTROY_MUTEX(mDqCvtLock);
}

OMX_ERRORTYPE MtkOmxVenc::ComponentInitDeque()
{
    IN_FUNC();
    mEncodeFlags = ENCODE_START;
    mDqFrameCount = 0;
    mDqBsCount = 0;
    mQueuedBsBuffers = 0;
    flushToEnd = false;
    bBsHasEos = false;

    int ret = pthread_create(&mVencDqThread, NULL, &MtkOmxVencDequeThread, (void *)this);
    if(ret) {
        MTK_OMX_LOGE("MtkOmxVencDequeThread creation failure");
        OUT_FUNC();
        return OMX_ErrorInsufficientResources;
    }

    OUT_FUNC();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::ComponentDeinitDeque()
{
    IN_FUNC();

    assert(mIsComponentAlive == OMX_FALSE);

    mEncodeFlags |= ENCODE_EOS;

    startFlush(true);
    SetPollInterrupt(); // send interrupt to dq thread in-case in poll
    if(!pthread_equal(pthread_self(), mVencDqThread))
    {
        LOCK(mDqFrmLock);
        SIGNAL_COND(mDqFrmCond);
        UNLOCK(mDqFrmLock);
        pthread_join(mVencDqThread, NULL);
    }

    OUT_FUNC();

    return OMX_ErrorNone;
}

void MtkOmxVenc::waitIfNoEnqueued()
{
    LOCK(mDqFrmLock);
    if(mFrmBufferTable.isEmpty() && (mQueuedBsBuffers == 0) && !isFlushingState() && !isEosState())
    {
        MTK_OMX_LOGV_ENG("Frame table empty, wait cond");
        WAIT_COND(mDqFrmCond, mDqFrmLock);
    }
    UNLOCK(mDqFrmLock);
}

OMX_BOOL MtkOmxVenc::startFlush(bool end)
{
    return startFlush(NULL, end);
}

OMX_BOOL MtkOmxVenc::startFlush(OMX_BUFFERHEADERTYPE* pInputBuf, bool end)
{
    IN_FUNC();
    if(end) MTK_OMX_LOGV_ENG(" --> Flush to end");

    onStartFlushBuffer(pInputBuf);
    flushToEnd = end;

    mEncodeFlags |= ENCODE_FLUSHING;

    if(end) SIGNAL(mEncodeSem);

    SIGNAL_COND(mWaitFTBCond); // avoid Encode Thread wait for buffer
    SetPollInterrupt(); // avoid Dq Thread poll
    SIGNAL_COND(mDqFrmCond); // avoid Dq Thread wait for Q event

    OUT_FUNC();
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::waitFlushDone()
{
    IN_FUNC();
    // Avoid Deque Thread blocking
    assert(gettid() != mVencDqThreadTid);

    MTK_OMX_LOGD("+ waitFlushDone mEncodeFlags %X mEncoderInitCompleteFlag %d mIsComponentAlive %d", mEncodeFlags, mEncoderInitCompleteFlag, mIsComponentAlive);
    while(mIsComponentAlive && mEncoderInitCompleteFlag && ( !isFlushDoneState() && !isEosState() ))
    {
        timespec now;
        clock_gettime(CLOCK_REALTIME, &now);
        now.tv_sec += 3;

        pthread_cond_timedwait(&mFlushDoneCond, &mFlushDoneCondLock, &now);
    }
    MTK_OMX_LOGD("- waitFlushDone mEncodeFlags %X mEncoderInitCompleteFlag %d mIsComponentAlive %d", mEncodeFlags, mEncoderInitCompleteFlag, mIsComponentAlive);

    OUT_FUNC();
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVenc::notifyFlushDone()
{
    IN_FUNC();

    // EOS Frame received
    // Buffers are requeued before here
    notifyFlushDoneBuffer();

    MTK_OMX_LOGD_ENG("mEncodeFlags %x", mEncodeFlags);

    mEncodeFlags &= (~ENCODE_FLUSHING);

    if(flushToEnd)
    {
        mEncodeFlags |= (ENCODE_EOS);
        SIGNAL(mEncodeSem);
    }

    MTK_OMX_LOGD_ENG("mEncodeFlags %x", mEncodeFlags);

    SIGNAL_COND(mFlushDoneCond);

    OUT_FUNC();
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::startDeque(int i_idx, OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8 *pCvtBuf, int o_idx, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    MTK_OMX_LOGV_ENG("Queue: In: [%d] %p, Cvt: [%d] %p, Out: [%d] %p", i_idx, pInputBuf, i_idx, pCvtBuf, o_idx, pOutputBuf);

    OMX_U8* ori = NULL;

    if(pCvtBuf != NULL && i_idx >= 0) {
        LOCK(mDqCvtLock);

        int ori_idx = mCvtBufferTable.indexOfKey(i_idx);
        if(ori_idx > 0) {
            ori = mCvtBufferTable.valueAt(ori_idx);
            mCvtBufferTable.removeItemsAt(ori_idx);
        }

        mCvtBufferTable.add(i_idx, pCvtBuf);
        UNLOCK(mDqCvtLock);
    }

    if(ori != NULL) MTK_OMX_LOGW("[WARNING][CONV] Convert Buffer %p replaced to %p", ori, pCvtBuf);

    return startDeque(i_idx, pInputBuf, o_idx, pOutputBuf);
}

OMX_BOOL MtkOmxVenc::startDeque(int i_idx, OMX_BUFFERHEADERTYPE *pInputBuf, int o_idx, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    OMX_BUFFERHEADERTYPE* ori = NULL;
    if(i_idx >= 0) {
        LOCK(mDqFrmLock);

        int ori_idx = mFrmBufferTable.indexOfKey(i_idx);
        if(ori_idx > 0) {
            ori = mFrmBufferTable.valueAt(ori_idx);
            mFrmBufferTable.removeItemsAt(ori_idx);
        }

        mFrmBufferTable.add(i_idx, pInputBuf);
        SIGNAL_COND(mDqFrmCond);
        UNLOCK(mDqFrmLock);

        onStartDequeBuffer(pInputBuf);
    }

    if(ori != NULL)
    {
        MTK_OMX_LOGW("[WARNING][FRAME] Frame Buffer %p replaced to %p", ori, pInputBuf);
        HandleEmptyBufferDone(ori);
    }

    /*if(pOutputBuf != NULL && o_idx >= 0) {
        LOCK(mDqBsLock);
        mBsBufferTable.add(o_idx, pOutputBuf);
        UNLOCK(mDqBsLock);
    }*/

    return startDeque();
}

OMX_BOOL MtkOmxVenc::startDeque()
{
    PROP();
    SIGNAL_COND(mDqFrmCond);
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::doDeque()
{
    IN_FUNC();

    OMX_BOOL ret = OMX_FALSE;
    OMX_BUFFERHEADERTYPE *pInputBuf = 0;
    OMX_BUFFERHEADERTYPE *pOutputBuf = 0;

    if(isFlushingState())
    {
        if( OMX_TRUE == tryNotifyFlushDone(NULL, NULL))
        {
            OUT_FUNC();
            return OMX_TRUE;
        }
    }

    if(OMX_FALSE == doDequeFrame(&pInputBuf, &pOutputBuf))
    {
        if(pInputBuf || pOutputBuf) {
            HandleDequeError(pInputBuf, pOutputBuf);
            //HandleFillBufferDone(pOutputBuf);
        }
        startDeque();
        OUT_FUNC(); return OMX_FALSE;
    }

    tryNotifyFlushDone(pInputBuf, pOutputBuf);

    OUT_FUNC(); return OMX_TRUE;

}

OMX_BOOL MtkOmxVenc::tryNotifyFlushDone(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    if( OMX_FALSE == shouldNotifyFlushDone(pInputBuf, pOutputBuf) )
    {
        return OMX_FALSE;
    }

    mEmptyThisBufQ.Lock(); // todo: workaround
    mFillThisBufQ.Lock(); // todo: workaround

    requeueBsBuffers();
    requeueFrameBuffers();

    MTK_OMX_LOGV_ENG("AFTER REQUEUE: FrameCount=%d/%d/%d/%d BsCount=%d/%d/%d/%d",
        mETBCount, mFrameCount, mDqFrameCount, mEBDCount, mFTBCount, mBsCount, mDqBsCount, mFBDCount);

    notifyFlushDone();

    mFillThisBufQ.Unlock(); // todo: workaround
    mEmptyThisBufQ.Unlock(); // todo: workaround
    return OMX_TRUE;
}

bool MtkOmxVenc::isFlushingState()
{
    return (mEncodeFlags & ENCODE_FLUSHING);
}

bool MtkOmxVenc::isFlushDoneState()
{
    return (mEncodeFlags & ENCODE_FLUSHING) == 0;
}

bool MtkOmxVenc::isEosState()
{
    return (mEncodeFlags & ENCODE_EOS);
}

bool MtkOmxVenc::isFatalError()
{
    return (mEncodeFlags & ENCODE_FATAL_ERR);
}

void MtkOmxVenc::HandleDequeError(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    if((pInputBuf && pInputBuf->nFlags & OMX_BUFFERFLAG_DATACORRUPT)
        || (pOutputBuf && pOutputBuf->nFlags & OMX_BUFFERFLAG_DATACORRUPT))
    {
        MTK_OMX_LOGE("## ENCODE ERROR: f(%X, %X)",
            (pInputBuf)?pInputBuf->nFlags:0,
            (pOutputBuf)?pOutputBuf->nFlags:0);

        CommonErrorHandle(OMX_ErrorStreamCorrupt, NULL);
    }
}

OMX_BOOL MtkOmxVenc::doDequeFrame(OMX_BUFFERHEADERTYPE **ppInputBuf, OMX_BUFFERHEADERTYPE **ppOutputBuf)
{
    IN_FUNC();

    MtkVenc::Profiler<int> prof;

    bool shouldNotifyFlushDone = true;

    // -1-: error or error event, 0: timeout, 1: dq, 2:event
    int event = 0;
    bool input_event = false;
    bool output_event = false;
    int ret = IoctlPoll(-1, &event, &input_event, &output_event);

    if(ret < 0)
    {
        MTK_OMX_LOGW("Poll Error: errno %d %s", errno, strerror(errno));
        mEncodeFlags = ENCODE_EOS;
        CommonErrorHandle(OMX_ErrorStreamCorrupt, NULL);
        shouldNotifyFlushDone = true; // not go to notify Flush done after here
    }
    /*
    if(ret == 0)
    {
        // poll timeout
        MTK_OMX_LOGW("Poll Timeout");
        shouldNotifyFlushDone = false;

        if(isFlushingState() && flushToEnd && isFatalError())
        {
            MTK_OMX_LOGW("[WARNING] Something happened to driver.. halt encode");
            mEncodeFlags = ENCODE_EOS;
            shouldNotifyFlushDone = true; // go to notify Flush done after here
        }
    }
    */
    if(input_event || output_event)
    {
        if(output_event)
        {
            shouldNotifyFlushDone &= (OMX_FALSE != doDequeOutputFrame(ppOutputBuf));
        }
        if(input_event)
        {
            shouldNotifyFlushDone &= (OMX_FALSE != doDequeInputFrame(ppInputBuf));
        }

        if(prof.doProfile && ppOutputBuf && *ppOutputBuf && ppInputBuf && *ppInputBuf)
        {
            prof.profile(1);

            MTK_OMX_LOGD_ENG("%s DqTime=%lld ms, FrameCount=%d/%d/%d/%d BsCount=%d/%d/%d/%d, bs ts=%llu (%llu), "
                "BS: VA=0x%08X, len=%lu, offset=0x%08x, flags=0x%08x Key(%d) Eos(%d) Corrupted(%d),"
                "FM: VA=0x%08X, len=%lu, offset=0x%08x, flags=0x%08x Key(%d) Eos(%d) Corrupted(%d)",
                codecName(), prof.tagDiff(1), mETBCount, mFrameCount, mDqFrameCount, mEBDCount,
                mFTBCount, mBsCount, mDqBsCount, mFBDCount, (*ppOutputBuf)->nTimeStamp / 1000, (*ppOutputBuf)->nTimeStamp,
                (*ppOutputBuf)->pBuffer, (*ppOutputBuf)->nFilledLen, (*ppOutputBuf)->nOffset, (unsigned int)(*ppOutputBuf)->nFlags,
                (*ppOutputBuf)->nFlags & OMX_BUFFERFLAG_SYNCFRAME, (*ppOutputBuf)->nFlags & OMX_BUFFERFLAG_EOS, (*ppOutputBuf)->nFlags & OMX_BUFFERFLAG_DATACORRUPT,
                (*ppInputBuf)->pBuffer, (*ppInputBuf)->nFilledLen, (*ppInputBuf)->nOffset, (unsigned int)(*ppInputBuf)->nFlags,
                (*ppInputBuf)->nFlags & OMX_BUFFERFLAG_SYNCFRAME, (*ppInputBuf)->nFlags & OMX_BUFFERFLAG_EOS, (*ppInputBuf)->nFlags & OMX_BUFFERFLAG_DATACORRUPT);
        }
        else if(prof.doProfile && ppOutputBuf && *ppOutputBuf)
        {
            prof.profile(1);

            MTK_OMX_LOGD_ENG("%s DqTime=%lld ms, FrameCount=%d/%d/%d/%d BsCount=%d/%d/%d/%d, bs ts=%llu (%llu), "
                "BS: VA=0x%08X, len=%lu, offset=0x%08x, flags=0x%08x Key(%d) Eos(%d) Corrupted(%d)",
                codecName(), prof.tagDiff(1), mETBCount, mFrameCount, mDqFrameCount, mEBDCount,
                mFTBCount, mBsCount, mDqBsCount, mFBDCount, (*ppOutputBuf)->nTimeStamp / 1000, (*ppOutputBuf)->nTimeStamp,
                (*ppOutputBuf)->pBuffer, (*ppOutputBuf)->nFilledLen, (*ppOutputBuf)->nOffset, (unsigned int)(*ppOutputBuf)->nFlags,
                (*ppOutputBuf)->nFlags & OMX_BUFFERFLAG_SYNCFRAME, (*ppOutputBuf)->nFlags & OMX_BUFFERFLAG_EOS, (*ppOutputBuf)->nFlags & OMX_BUFFERFLAG_DATACORRUPT);
        }
        else if(prof.doProfile && ppInputBuf && *ppInputBuf)
        {
            prof.profile(1);

            MTK_OMX_LOGD_ENG("%s DqTime=%lld ms, FrameCount=%d/%d/%d/%d BsCount=%d/%d/%d/%d, fm ts=%llu (%llu), "
                "FM: VA=0x%08X, len=%lu, offset=0x%08x, flags=0x%08x Key(%d) Eos(%d) Corrupted(%d)",
                codecName(), prof.tagDiff(1), mETBCount, mFrameCount, mDqFrameCount, mEBDCount,
                mFTBCount, mBsCount, mDqBsCount, mFBDCount, (*ppInputBuf)->nTimeStamp / 1000, (*ppInputBuf)->nTimeStamp,
                (*ppInputBuf)->pBuffer, (*ppInputBuf)->nFilledLen, (*ppInputBuf)->nOffset, (unsigned int)(*ppInputBuf)->nFlags,
                (*ppInputBuf)->nFlags & OMX_BUFFERFLAG_SYNCFRAME, (*ppInputBuf)->nFlags & OMX_BUFFERFLAG_EOS, (*ppInputBuf)->nFlags & OMX_BUFFERFLAG_DATACORRUPT);
        }
    }

    if(ret == 2)
    {
        // handle event here
    }

    OUT_FUNC();
    return (shouldNotifyFlushDone)?OMX_TRUE:OMX_FALSE;
}

OMX_BOOL MtkOmxVenc::doDequeInputFrame(OMX_BUFFERHEADERTYPE **ppInputBuf)
{
    IN_FUNC();

    if(mFrameCount == mDqFrameCount) {
        OUT_FUNC();
        return OMX_TRUE;
    }

    OMX_U8* pCvtBuf = 0;
    int in_ret = ioctl_dq_in_buf(ppInputBuf, &pCvtBuf);

    if(in_ret == -1) {
        OUT_FUNC();
        return OMX_FALSE;
    }

    mDqFrameCount ++;

    if(ppInputBuf == NULL || *ppInputBuf == NULL) {
        OUT_FUNC();
        return OMX_FALSE;
    }

    OMX_BUFFERHEADERTYPE *pInputBuf = *ppInputBuf;

    epilogueDequeInputFrame(pInputBuf, pCvtBuf);

    if(pInputBuf == NULL) return OMX_FALSE;

    HandleEmptyBufferDone(pInputBuf);

    OUT_FUNC();
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::doDequeOutputFrame(OMX_BUFFERHEADERTYPE **ppOutputBuf)
{
    IN_FUNC();

    if(mBsCount == mDqBsCount) {
        OUT_FUNC();
        return OMX_TRUE;
    }

    int out_ret = ioctl_dq_out_buf(ppOutputBuf);

    if(out_ret == -1) {
        OUT_FUNC();
        return OMX_FALSE;
    }

    mDqBsCount ++;

    if(ppOutputBuf == NULL || *ppOutputBuf == NULL) {
        OUT_FUNC();
        return OMX_FALSE;
    }

    OMX_BUFFERHEADERTYPE *pOutputBuf = *ppOutputBuf;
    OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
    OMX_U32 aOutputSize = pOutputBuf->nAllocLen;

    dumpOutputBuffer(pOutputBuf, aOutputBuf, pOutputBuf->nFilledLen);

    epilogueDequeOutputFrame(pOutputBuf);

    HandleFillBufferDone(pOutputBuf);

    WFD_HandleDequeFrame(pOutputBuf);

    OUT_FUNC();
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::shouldNotifyFlushDone(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    IN_FUNC();

    // just mark bs buffer eos flag received, but keep deque to get B-frame left
    if(pOutputBuf && pOutputBuf->nFlags & OMX_BUFFERFLAG_EOS)
    {
        MTK_OMX_LOGD_ENG("[%s] BS EOS received", __func__);
        bBsHasEos = true;
    }

    // notify flush end if eos state
    if( isEosState() )
    {
        MTK_OMX_LOGD_ENG("[%s] Component not alive. State: %x", __func__, mEncodeFlags);
        OUT_FUNC();
        return OMX_TRUE;
    }

    // Conditions below are needed to be Flushing State
    if(!isFlushingState())
    {
        OUT_FUNC();
        return OMX_FALSE;
    }

    // Non Flushing State
    if(pInputBuf && pInputBuf->nFlags & OMX_BUFFERFLAG_EOS)
    {
        MTK_OMX_LOGD_ENG("[%s] Frame EOS received", __func__);
        flushToEnd = true;
        OUT_FUNC();
        return OMX_TRUE;
    }

    MTK_OMX_LOGD("NOTIFY:  FrameCount=%d/%d/%d/%d BsCount=%d/%d/%d/%d mEncodeFlags 0x%x",
        mETBCount, mFrameCount, mDqFrameCount, mEBDCount, mFTBCount, mBsCount, mDqBsCount, mFBDCount, mEncodeFlags);

    // shouldNotifyFlushDone with output buffer
    if( OMX_TRUE == shouldNotifyFlushDoneBuffer(pInputBuf, pOutputBuf))
    {
        MTK_OMX_LOGD_ENG("[%s] shouldNotifyFlushDoneBuffer", __func__);
        OUT_FUNC();
        return OMX_TRUE;
    }

    // Flush to end and EOS
    if(mDqFrameCount == mFrameCount && flushToEnd && bBsHasEos)
    {
        MTK_OMX_LOGD_ENG("[%s] Enq == Deq count + Flush to eos + BS has EOS", __func__);
        OUT_FUNC();
        return OMX_TRUE;
    }

    // just simply Flush, and will continue encode
    if(mDqFrameCount == mFrameCount && !flushToEnd)
    {
        MTK_OMX_LOGD_ENG("[%s] Enq == Deq count + not Flush to eos", __func__);
        OUT_FUNC();
        return OMX_TRUE;
    }

    if(mDqFrameCount == mFrameCount && mDqBsCount == mBsCount  && flushToEnd)
    {
        OUT_FUNC();
        return OMX_TRUE;
    }

    OUT_FUNC();
    return OMX_FALSE;
}

OMX_BOOL MtkOmxVenc::requeueFrameBuffers()
{
    IN_FUNC();

    ioctl_stream_off(1);

    for(int index=0; index < EncodeT_Input_GetBufferCountActual(); index++)
    {
        OMX_BUFFERHEADERTYPE* pInputBuf = NULL;

        LOCK(mDqFrmLock);
        pInputBuf = mFrmBufferTable.valueFor(index);
        UNLOCK(mDqFrmLock);

        if(pInputBuf != NULL)
        {
            MTK_OMX_LOGW("%s: found %p at index %d", __func__, pInputBuf, index);
            ++mDqFrameCount;
            HandleEmptyBufferDone(pInputBuf);
            //QueueInputBuffer(index);
        }
    }

    // fixing Q and Dq frame count by max
    mFrameCount = (mFrameCount>mDqFrameCount)?mFrameCount:mDqFrameCount;
    mDqFrameCount = (mFrameCount>mDqFrameCount)?mFrameCount:mDqFrameCount;

    if(!flushToEnd) ioctl_stream_on(1);

    OUT_FUNC();
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::requeueBsBuffers()
{
    IN_FUNC();

    ioctl_stream_off(2);

    int index = 0;

    for(int index = 0; index < EncodeT_Output_GetBufferCountActual(); index++)
    {
        OMX_BUFFERHEADERTYPE* pOutputBuf = NULL;

        if( mQueuedBsBuffers & (1 << index) )
        {
            pOutputBuf = EncodeT_Output_GetBufferHeaderPtr(index);
            mQueuedBsBuffers &= ~(1 << index);
        }

        if(pOutputBuf != NULL)
        {
            ++mDqBsCount;
            if(stop_cmd_sent == 1) pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
            HandleFillBufferDone(pOutputBuf);
            //QueueOutputBuffer(index);
        }
    }

    // fixing Q and Dq bs count by max
    mBsCount = (mBsCount>mDqBsCount)?mBsCount:mDqBsCount;
    mDqBsCount = (mBsCount>mDqBsCount)?mBsCount:mDqBsCount;

    if(!flushToEnd) ioctl_stream_on(2);

    OUT_FUNC();
    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::epilogueDequeOutputFrame(OMX_BUFFERHEADERTYPE *pOutputBuf)
{
    if(pOutputBuf == NULL) return OMX_FALSE;

    // workaround tickCount
    pOutputBuf->nTickCount = mDqBsCount;

    int index = EncodeT_Output_GetBufferIndex(pOutputBuf);

    if(index >= 0) mQueuedBsBuffers &= ~(1 << index);

    if(pOutputBuf->nFlags & OMX_BUFFERFLAG_DATACORRUPT)
    {
        HandleDequeError(NULL, pOutputBuf);
    }

    if(isFlushingState() && flushToEnd && mDqBsCount == mBsCount)
    {
        pOutputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
    }

    MTK_OMX_LOGV_ENG("EncodeFunc Done (%d), Out=%p", mDqBsCount, pOutputBuf);

    OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
    ClearVEncDrvBSBuffer(aOutputBuf);

    return OMX_TRUE;
}

OMX_BOOL MtkOmxVenc::epilogueDequeInputFrame(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8* pCvtBuf)
{
    MTK_OMX_LOGV_ENG("EncodeFunc Done (%d), In=%p Cvt=%p", mDqFrameCount, pInputBuf, pCvtBuf);

    OMX_U8 *aInputBuf = NULL;
    if(pInputBuf)
    {
        aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;
        pInputBuf->pPlatformPrivate = NULL; // clear convert buffer
    }

    ClearVEncDrvFrmBuffer(aInputBuf, pCvtBuf);

    if(pInputBuf && pInputBuf->nFlags & OMX_BUFFERFLAG_DATACORRUPT)
    {
        HandleDequeError(pInputBuf, NULL);
    }

    return OMX_TRUE;
}

void MtkOmxVenc::DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf, void* dqbuf)
{
    IN_FUNC();
    v4l2_buffer* buf = (v4l2_buffer*)dqbuf;

    OMX_BUFFERHEADERTYPE* pInputBuf = NULL;

    LOCK(mDqFrmLock);

    int ori_idx = -1;
    do {
        ori_idx = mFrmBufferTable.indexOfKey(buf->index);
        if(ori_idx < 0)
        {
            MTK_OMX_LOGW("Index %d not yet in frame table", buf->index);
            WAIT_COND(mDqFrmCond, mDqFrmLock);
        }
    } while(ori_idx < 0);

    pInputBuf = mFrmBufferTable.valueAt(ori_idx);
    mFrmBufferTable.removeItemsAt(ori_idx);

    UNLOCK(mDqFrmLock);

    MTK_OMX_LOGV_ENG("Get input %p from index %d", pInputBuf, buf->index);

    if(pInputBuf == NULL) return;

    if(buf->flags & V4L2_BUF_FLAG_ERROR)
    {
        pInputBuf->nFlags |= OMX_BUFFERFLAG_DATACORRUPT;
    }

    if(buf->flags & V4L2_BUF_FLAG_LAST)
    {
        pInputBuf->nFlags |= OMX_BUFFERFLAG_EOS;
    }
    else
    {
        pInputBuf->nFlags &= ~OMX_BUFFERFLAG_EOS;
    }

    // EXT: DequeOnGetInputBuffer
    ROI_DequeOnGetInputBuffer(pInputBuf, (v4l2_buffer*)dqbuf);
    Heif_DequeOnGetInputBuffer(pInputBuf, (v4l2_buffer*)dqbuf);

    *ppInputBuf = pInputBuf;

    OUT_FUNC();
    return;
}

void MtkOmxVenc::DequeOnGetConvertBuffer(OMX_U8** ppCvtBuf, void* dqbuf)
{
    IN_FUNC();
    v4l2_buffer* buf = (v4l2_buffer*)dqbuf;

    OMX_U8 *pCvtBuf = NULL;

    LOCK(mDqCvtLock);
    pCvtBuf = mCvtBufferTable.valueFor(buf->index);
    mCvtBufferTable.removeItem(buf->index);
    UNLOCK(mDqCvtLock);

    MTK_OMX_LOGV_ENG("Get cnvt %p from index %d", pCvtBuf, buf->index);

    if(pCvtBuf == NULL) return;

    *ppCvtBuf = pCvtBuf;

    OUT_FUNC();
    return;
}

void MtkOmxVenc::DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf, void* dqbuf)
{
    IN_FUNC();
    v4l2_buffer* buf = (v4l2_buffer*)dqbuf;

    OMX_BUFFERHEADERTYPE* pOutputBuf = NULL;
    pOutputBuf = EncodeT_Output_GetBufferHeaderPtr(buf->index);

    /*LOCK(mDqBsLock);
    pOutputBuf = mBsBufferTable.valueFor(buf->index);
    mBsBufferTable.add(buf->index, NULL);
    UNLOCK(mDqBsLock);*/

    MTK_OMX_LOGV_ENG("Get output %p from index %d", pOutputBuf, buf->index);

    if(pOutputBuf == NULL) return;

    Buffer_DequeOnGetOutputBuffer(pOutputBuf, dqbuf);
    WFD_DequeOnGetOutputBuffer(pOutputBuf, dqbuf);
    SMVR_DequeOnGetOutputBuffer(pOutputBuf, dqbuf);
    ROI_DequeOnGetOutputBuffer(pOutputBuf, dqbuf);
    Heif_DequeOnGetOutputBuffer(pOutputBuf, (v4l2_buffer*)dqbuf);

    *ppOutputBuf = pOutputBuf;

    OUT_FUNC();
    return;
}