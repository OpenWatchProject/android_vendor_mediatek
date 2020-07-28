#include <signal.h>
#include <cutils/log.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

#include "osal_utils.h"
#include "MtkOmxVenc.h"

#include <cutils/properties.h>
#include <sched.h>
#include <assert.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVenc"

#include "OMX_IndexExt.h"

#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include "videodev2.h"
#include "v4l2-controls.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

// Morris Yang 20120214 add for live effect recording [
#ifdef ANDROID_ICS
#include <ui/Rect.h>
//#include <ui/android_native_buffer.h> // for ICS
#include <android/native_window.h> // for JB
// #include <media/stagefright/HardwareAPI.h> // for ICS
#include <HardwareAPI.h> // for JB
//#include <media/stagefright/MetadataBufferType.h> // for ICS
#include <MetadataBufferType.h> // for JB

#include <hardware/gralloc.h>
#include <ui/gralloc_extra.h>
#endif
// ]
#define OMX_CHECK_DUMMY
#include "../../../omx/core/src/MtkOmxCore.h"

#include <poll.h>

#include <utils/Trace.h>
//#include <utils/AndroidThreads.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVencSMVR"

#define SUPPORT_SMVR

void MtkOmxVenc::MtkOmxVencSMVR()
{
    mOperationRate = 0;
    mSubFrameIndex = 0;
    mSubFrameTotalCount = 0;
    mSubFrameTimeStamp = 0;
    mSubFrameLastTimeStamp = 0;
    mLastFrameTimeStamp = 0;
    mSMVROn = OMX_FALSE;
    mFlushAll = OMX_FALSE;
    mCollectRate = 4;
    mSubFrameRate = 0;
}

void MtkOmxVenc::deMtkOmxVencSMVR()
{

}

OMX_BOOL MtkOmxVenc::SMVR_OnInit(video_encode_param* param)
{
    // Workaround for HeifWriterTest; it would set operation rate to 120
    if (mEncParam.width == 512 && mEncParam.height == 512)
        mOperationRate = 0;

#ifdef SUPPORT_SMVR
    if(mOperationRate >= 120)
    {
        mSubFrameRate = mOperationRate / mEncParam.framerate;
        mSMVROn = OMX_TRUE;
    }
    else
    {
        mSMVROn = OMX_FALSE;
    }

    if (mOperationRate > 0)
    {
        mBurstTicksPerFrame = 1000000UL/mOperationRate;
    }

    GET_SYSTEM_PROP(OMX_BOOL, mFlushAll, "vendor.mtk.omx.smvr.flushall", "0");
    GET_SYSTEM_PROP(OMX_U32, mCollectRate, "vendor.mtk.omx.smvr.collectRate", "4");

    mSMVRDumpInputFrame = (OMX_BOOL) MtkVenc::DumpInputFrame();

    if(mCollectRate > EncodeT_Output_GetBufferCountActual())
        mCollectRate = EncodeT_Output_GetBufferCountActual();

    MTK_OMX_LOGD_ENG("%s smvr %d opr %d (%d), collect rate %d",
        __func__, mSMVROn, mOperationRate, mSubFrameRate, mCollectRate);
#endif
    return mSMVROn;
}

OMX_ERRORTYPE MtkOmxVenc::SMVR_SetMode()
{
    if (mOperationRate != 0)
    {
        unsigned int operationrate = mOperationRate;
        //rRet = eVEncDrvSetParam(mDrvHandle, VENC_DRV_SET_TYPE_OPERATION_RATE, (VAL_VOID_T *)&operationrate, VAL_NULL);
        int nRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_OPERATION_RATE, operationrate);
        if (-1 == nRet)
        {
            MTK_OMX_LOGE("[ERROR] set venc operation rate(%d) fail", operationrate);
        }
        return OMX_ErrorNone;
    }
    return OMX_ErrorNotReady;
}

OMX_ERRORTYPE MtkOmxVenc::SMVR_SetConfigOperatingRate(OMX_PARAM_U32TYPE* pOperationRate)
{
    mOperationRate = (unsigned int)(pOperationRate->nU32 / 65536.0f);
    MTK_OMX_LOGD_ENG("MtkOmxVenc::SetConfig ->operation rate set to %u", mOperationRate);

    return OMX_ErrorNone;
}

void MtkOmxVenc::SMVR_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, buffer_handle_t handle, struct v4l2_buffer* qbuf)
{
    if(mSMVROn == OMX_FALSE) return;

#ifdef SUPPORT_SMVR
    if((unsigned int)handle != mQHandle) {
        gralloc_extra_query(handle, GRALLOC_EXTRA_GET_SMVR_INFO, &mQVrInfo);
        mQHandle = (unsigned int)handle;
    }

    return SMVR_UpdateInputV4L2Buffer(pInputBuf, &mQVrInfo, qbuf);
#endif
    return;
}

void MtkOmxVenc::SMVR_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, void* smvr, struct v4l2_buffer* qbuf)
{
    if(mOperationRate < 120) return;
    if(pInputBuf == NULL || qbuf == NULL) return;
    if(pInputBuf->nFlags & OMX_BUFFERFLAG_EOS && pInputBuf->nFilledLen == 0) return;

    ge_smvr_info_t* smvrInfo = (ge_smvr_info_t*)smvr;
    if(smvrInfo->frame_count <= 1) return;

    qbuf->flags |= V4L2_BUF_FLAG_NO_CACHE_CLEAN;

    if(mSMVROn == OMX_FALSE) return; // >= 240
    // Update Offset
    int planeSize = VENC_ROUND_N(mEncParam.width, 32) * VENC_ROUND_N(mEncParam.height, 32) * 3 >> 1;

    if(planeSize <= 0) return;

    int offset = planeSize * mSubFrameIndex;

    for(int i=0; i<qbuf->length; i++) {
        qbuf->m.planes[i].length += offset;
        qbuf->m.planes[i].bytesused += offset;
        qbuf->m.planes[i].data_offset += offset;
    }

    return;
}

void MtkOmxVenc::SMVR_UpdateOutputV4L2Buffer(OMX_BUFFERHEADERTYPE *pOutputBuf, struct v4l2_buffer* qbuf)
{
    if(mSMVROn == OMX_FALSE) return;
    if(pOutputBuf == NULL || qbuf == NULL) return;

    if(mFlushAll == OMX_TRUE) qbuf->flags |= V4L2_BUF_FLAG_NO_CACHE_INVALIDATE;
    // else use v4l flush mechanism on every frame

    return;
}

void MtkOmxVenc::SMVR_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf, buffer_handle_t handle)
{
    if(mSMVROn == OMX_FALSE) return;
    if (!ppInputBuf || !*ppInputBuf) return;
    OMX_BUFFERHEADERTYPE* pInputBuf = *ppInputBuf;

#ifdef SUPPORT_SMVR
    if((unsigned int)handle != mQHandle) {
        gralloc_extra_query(handle, GRALLOC_EXTRA_GET_SMVR_INFO, &mQVrInfo);
        mQHandle = (unsigned int)handle;
    }

    return SMVR_BufferModifyBeforeEnc(ppInputBuf, &mQVrInfo);
#endif
    return;
}

static void dumpSmvrTs(bool enabled, ge_smvr_info_t* smvrInfo)
{
    if(!enabled) return;

    int len = smvrInfo->frame_count;

    char ts_buf[256] = {0};

    for(int i=0; i<len; i++)
    {
        char tmp[32];
        snprintf(tmp, sizeof(tmp), ",%lld", smvrInfo->timestamp[i]);
        strncat(ts_buf, tmp, sizeof(ts_buf) - strlen(ts_buf) - 1);
    }

    ALOGD("DumpSmvrTs[%d]: %s", len, ts_buf+1);
}

#define SMVR_FRAMERATE_30 1
#define SMVR_FRAMERATE_120 4
#define SMVR_FRAMERATE_240 8
#define SMVR_FRAMERATE_480 16
#define SMVR_FRAMERATE_960 32

void MtkOmxVenc::SMVR_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf, void* smvr)
{
    if(mSMVROn == OMX_FALSE) return;
    if (!ppInputBuf || !*ppInputBuf) return;
    OMX_BUFFERHEADERTYPE* pInputBuf = *ppInputBuf;

#ifdef SUPPORT_SMVR
    ge_smvr_info_t* smvrInfo = (ge_smvr_info_t*)smvr;
    mSubFrameTimeStamp = smvrInfo->timestamp[mSubFrameIndex] / 1000UL;

    if(mSubFrameTimeStamp != 0)
    {
        if (mSubFrameLastTimeStamp != 0)
        {
            OMX_TICKS delta = (mSubFrameTimeStamp - mSubFrameLastTimeStamp);

/*
    for burst mode SMVR (Normal/High framerate)
    only adjust timestamp diff when high framerate range
    timestamp low 8 bit hint camera capture framerate
*/
            if(mOperationRate > 0 && !(smvrInfo->timestamp[mSubFrameIndex] & SMVR_FRAMERATE_30) )
            {
                delta = (delta>mBurstTicksPerFrame)?mBurstTicksPerFrame:delta;
                delta *= mSubFrameRate;
            }

            pInputBuf->nTimeStamp = mLastFrameTimeStamp + delta;
        }

        mLastFrameTimeStamp = pInputBuf->nTimeStamp;
        mSubFrameLastTimeStamp = mSubFrameTimeStamp;
    }
#endif
    dumpSmvrTs(mEnableMoreLog>=2, smvrInfo);

    MTK_OMX_LOGD_ENG("%s SubFrameLastTS %lld BufferTS %lld mSubFrameIndex: %d",
        __func__, mSubFrameLastTimeStamp, pInputBuf->nTimeStamp, mSubFrameIndex);

    return;
}

void MtkOmxVenc::SMVR_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf, buffer_handle_t handle)
{
    if(mSMVROn == OMX_FALSE) return;
#ifdef SUPPORT_SMVR
    if((unsigned int)handle != mQHandle) {
        gralloc_extra_query(handle, GRALLOC_EXTRA_GET_SMVR_INFO, &mQVrInfo);
        mQHandle = (unsigned int)handle;
    }
    return SMVR_OnEnqueBuffer(ppInputBuf, &mQVrInfo);
#endif
}

void MtkOmxVenc::SMVR_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf, void* smvr)
{
    if(mSMVROn == OMX_FALSE) return;
#ifdef SUPPORT_SMVR
    ge_smvr_info_t* smvrInfo = (ge_smvr_info_t*)smvr;
    if(smvrInfo->frame_count <= 1) return;

    mSubFrameTotalCount = smvrInfo->frame_count;

    if(mSubFrameTotalCount != 0)
    {
        // consider as one frame
        mSubFrameIndex++;
    }

    if (mSubFrameIndex < mSubFrameTotalCount)
    {
        QueueBufferAdvance(&mEmptyThisBufQ, *ppInputBuf);
        *ppInputBuf = 0; // Not going to deque
    }
    else
    {
        mSubFrameIndex -= mSubFrameTotalCount;
    }

    MTK_OMX_LOGD_ENG("%s Frame Index %d of %d",
        __func__, mSubFrameIndex, mSubFrameTotalCount);
#endif
}

void MtkOmxVenc::SMVR_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, void* dqbuf)
{
    if(mSMVROn == OMX_FALSE) return;
    if(pOutputBuf == NULL) return;
}

void MtkOmxVenc::SMVR_CollectBsBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf)
{
    if(mSMVROn == OMX_FALSE) return;
    if(ppOutputBuf == NULL || *ppOutputBuf == NULL) return;
    if(!mFlushAll) return; // No need collect flush

    VAL_UINT32_T mOutputBufferIndex = EncodeT_Output_GetBufferIndex(*ppOutputBuf);
    mCollectedBs.push(mOutputBufferIndex);

    *ppOutputBuf = NULL;
}

void MtkOmxVenc::SMVR_OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf)
{
    if(mSMVROn == OMX_FALSE) return;
    if(!mFlushAll) return; // No need collect flush

    // output buffer should be NULL
    assert(*ppOutputBuf == NULL);

    // queue collected buffer to driver either
    //  - on collect rate or
    //  - on flush to EOS state
    if(mCollectedBs.size() != mCollectRate && !(isFlushingState() && flushToEnd)) return;

    for(int i=0; i<mCollectRate; i++)
    {
        int output_idx = mCollectedBs[0];
        mCollectedBs.removeAt(0);

        OMX_BUFFERHEADERTYPE* pOutputBuf = EncodeT_Output_GetBufferHeaderPtr(output_idx);

        if(!pOutputBuf) continue;

        if(mFlushAll == OMX_TRUE && i == 0)
        {
            OMX_U8 *aOutputBuf = pOutputBuf->pBuffer + pOutputBuf->nOffset;
            mOutputMVAMgr->syncBufferCacheFrm(aOutputBuf, /*flush all*/ true);
        }

        PrepareOutputBuffer(pOutputBuf);
    }
}

void MtkOmxVenc::SMVR_DumpInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    if(mSMVROn == OMX_FALSE) return;

    aInputBuf += (aInputSize * mSubFrameIndex);
    MTK_OMX_LOGV_ENG("SMVR_DumpInputBuffer[%d]: 0x%x [%d]", mSubFrameIndex, aInputBuf, aInputSize);

    char name[128];
    int width = mInputPortDef.format.video.nFrameWidth;
    int height = mInputPortDef.format.video.nFrameHeight;

    if(OMX_TRUE == mSMVRDumpInputFrame)
    {
        if (OMX_FALSE == mDumpCts) {
            snprintf(name, sizeof(name), "/sdcard/smvr_input_%u_%u_%u_%d.yuv", width, height, aInputSize, gettid());
        } else {
            snprintf(name, sizeof(name), "/sdcard/vdump/Venc_smvr_input_%u_%u_%05d.yuv", width, height, gettid());
        }

        dumpBuffer(name, (unsigned char *)aInputBuf, aInputSize);
    }
}
