#include <signal.h>
#include <cutils/log.h>

#include <utils/Trace.h>
#include <utils/AndroidThreads.h>

#include "osal_utils.h"
#include "MtkOmxVenc.h"
#include "MtkOmx2V4L2.h"

#include <cutils/properties.h>
#include <sched.h>
#include <assert.h>

#undef LOG_TAG
#define LOG_TAG "MtkOmxVencWFD"

#include "OMX_IndexExt.h"

#ifdef ATRACE_TAG
#undef ATRACE_TAG
#define ATRACE_TAG ATRACE_TAG_VIDEO
#endif//ATRACE_TAG

#include <utils/Trace.h>
//#include <utils/AndroidThreads.h>

void MtkOmxVenc::MtkOmxVencWFD()
{
    mSetWFDMode = OMX_FALSE;
    mWFDMode = OMX_FALSE;
    mEnableDummy = (OMX_BOOL) MtkVenc::EnableDummy("1");

    INIT_MUTEX(mDummyFrmLock);
}

void MtkOmxVenc::deMtkOmxVencWFD()
{
    DESTROY_MUTEX(mDummyFrmLock);
}

OMX_ERRORTYPE MtkOmxVenc::WFD_EnableToggle()
{
    mSetWFDMode = OMX_TRUE;
    mWFDMode = OMX_TRUE;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_DisableToggle()
{
    mSetWFDMode = OMX_FALSE;
    mWFDMode = OMX_FALSE;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_SetMode()
{
    int rRet = 0;

    if (mSetWFDMode != OMX_TRUE) return OMX_ErrorNotReady;

    mSetWFDMode = OMX_FALSE;
    mWFDMode = OMX_TRUE;

    rRet = ioctl_runtime_config(V4L2_CID_MPEG_MTK_ENCODE_SCENARIO, 1);
    if (-1 == rRet)
    {
        MTK_OMX_LOGE("[ERROR] set WFD mode fail");
        mWFDMode = OMX_FALSE;
    }

    if(!mEnableDummy) return OMX_ErrorNone;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam)
{
    OMX_ERRORTYPE err = OMX_ErrorNone;

    switch (nParamIndex)
    {
        case OMX_IndexParamVideoBitrate:
            err = HandleWFDSetVideoBitrate((OMX_VIDEO_PARAM_BITRATETYPE *)pCompParam);
            break;
        default:
            err = OMX_ErrorUnsupportedIndex;
    }
    return err;
}

OMX_ERRORTYPE MtkOmxVenc::HandleWFDSetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE* pBitrateType)
{
    if (pBitrateType->eControlRate == OMX_Video_ControlRateMtkWFD)
    {
        WFD_EnableToggle();
    }
    else
    {
        WFD_DisableToggle();
    }

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    if (mSetWFDMode != OMX_TRUE) return OMX_ErrorNotReady;
    if (!ppInputBuf || !*ppInputBuf) return OMX_ErrorNotReady;
    OMX_BUFFERHEADERTYPE* pInputBuf = *ppInputBuf;

    if (OMX_TRUE == mStoreMetaDataInBuffers)
    {
        OMX_U8 *aInputBuf   = pInputBuf->pBuffer + pInputBuf->nOffset;
        OMX_U32 _handle = 0;
        GetMetaHandleFromBufferPtr(aInputBuf, &_handle);

        gralloc_extra_ion_sf_info_t ext_info;
        gralloc_extra_query((buffer_handle_t)_handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &ext_info);
        //MTK_OMX_LOGD("get sequence info: %u", ext_info.sequence);
        pInputBuf->nTickCount = (OMX_U32)ext_info.sequence;

        return OMX_ErrorNone;
    }
    return OMX_ErrorNotReady;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_DrawYUVStripeLine(
    unsigned char** pY, unsigned char** pU, unsigned char** pV, OMX_U32 iDrawWidth, OMX_U32 iDrawHeight
    )
{
    if (OMX_TRUE == mWFDMode)
    {
        *pU = *pY + VENC_ROUND_N(iDrawWidth, 16) * VENC_ROUND_N(iDrawHeight, 16);
        *pV = *pU + ((VENC_ROUND_N((iDrawWidth >> 1), 16) * VENC_ROUND_N(iDrawHeight, 16) >> 1));
        //pU = pY+VENC_ROUND_N(iDrawWidth, 16)*VENC_ROUND_N(iDrawHeight, 32);
        //pV = pU+((VENC_ROUND_N((iDrawWidth>>1), 16)*VENC_ROUND_N(iDrawHeight, 32)>>1));

        return OMX_ErrorNone;
    }

    return OMX_ErrorNotReady;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf)
{
    if (mWFDMode != OMX_TRUE || mEnableDummy != OMX_TRUE) return OMX_ErrorNotReady;
    if (ppOutputBuf == NULL || *ppOutputBuf == NULL) return OMX_ErrorBadParameter;

    int dummyIdx = DequeueBufferAdvance(&mFillThisBufQ);

    if(dummyIdx < 0) return OMX_ErrorNotReady;

    int outputBufferIndex = EncodeT_Output_GetBufferIndex(*ppOutputBuf);
    OMX_BUFFERHEADERTYPE* pDummyOutputBufHdr = EncodeT_Output_GetBufferHeaderPtr(dummyIdx);

    LOCK(mDummyFrmLock);
    mDummyBufferTable.add(outputBufferIndex, pDummyOutputBufHdr);
    UNLOCK(mDummyFrmLock);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBufHdr, void* dqbuf)
{
    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::WFD_HandleDequeFrame(OMX_BUFFERHEADERTYPE* pOutputBufHdr)
{
    if (mWFDMode != OMX_TRUE) return OMX_ErrorNotReady;
    if (pOutputBufHdr == NULL) return OMX_ErrorNotReady;

    int outputBufferIndex = EncodeT_Output_GetBufferIndex(pOutputBufHdr);

    LOCK(mDummyFrmLock);
    OMX_BUFFERHEADERTYPE* pDummyOutputBufHdr = mDummyBufferTable.valueFor(outputBufferIndex);
    mDummyBufferTable.add(outputBufferIndex, NULL);
    UNLOCK(mDummyFrmLock);

    if(pDummyOutputBufHdr == NULL)
        return OMX_ErrorNotReady;

    OMX_U8 *aOutputBuf = pDummyOutputBufHdr->pBuffer + pDummyOutputBufHdr->nOffset;

    /*if (mStoreMetaDataInOutBuffers)
    {
        if(1) // SetBitstreamSize4Framework
        {
            int *bitstreamLen = (int*)(pDummyOutputBufHdr + sizeof(struct VideoNativeHandleMetadata));
            *bitstreamLen = pDummyOutputBufHdr->nFilledLen;
            pDummyOutputBufHdr->nFilledLen = 8 + 4;
        }
    }*/

    if(!mStoreMetaDataInOutBuffers)
    {
        GenerateDummyBuffer(pDummyOutputBufHdr);
    }

    pDummyOutputBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
    pDummyOutputBufHdr->nFlags |= OMX_BUFFERFLAG_DUMMY_NALU;
    pDummyOutputBufHdr->nTickCount = pOutputBufHdr->nTickCount;
    pDummyOutputBufHdr->nTimeStamp = pOutputBufHdr->nTimeStamp;

    dumpOutputBuffer(pDummyOutputBufHdr, aOutputBuf, pDummyOutputBufHdr->nFilledLen);
    HandleFillBufferDone(pDummyOutputBufHdr);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE MtkOmxVenc::GenerateDummyBuffer(OMX_BUFFERHEADERTYPE* pDummyOutputBufHdr)
{
    switch (mEncParam.codec)
    {
        case V4L2_PIX_FMT_H264:
            GenerateDummyBufferH264Enc(pDummyOutputBufHdr);
            break;
        default:
            pDummyOutputBufHdr->nFilledLen = 0;
            return OMX_ErrorNotReady;
    }

    return OMX_ErrorNone;
}
